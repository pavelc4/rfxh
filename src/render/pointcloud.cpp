#include "render/render.hpp"
#include "render/constants.hpp"
#include "logo/logo.hpp"
#include "text/charweight.hpp"
#include "common/types.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace rfxh::render {

void RenderEngine::clear_buf(int render_height) {
    int n = render_height * kFrameWidth;
    std::memset(shade_idx.data(), -1, n);
    std::memset(zbuf.data(), 0, n * sizeof(float));
    std::memset(colorbuf.data(), 0, n * sizeof(int));
}

void RenderEngine::build_points(const logo::Logo& logo, float size_scale) {
    const float sx   = 0.07f;
    const float sy   = 0.14f;
    const float cx   = (logo.cols - 1) * 0.5f;
    const float cy   = (logo.rows - 1) * 0.5f;
    const float zmax = 0.18f;

    int z_layers = static_cast<int>(6 * size_scale);
    if (z_layers < 6)
        z_layers = 6;

    std::vector<std::array<float, kMaxLogoCols>> hmap(kMaxLogoRows);
    std::vector<std::array<float, kMaxLogoCols>> gnx(kMaxLogoRows);
    std::vector<std::array<float, kMaxLogoCols>> gny(kMaxLogoRows);
    std::vector<std::array<float, kMaxLogoCols>> gnz(kMaxLogoRows);

    for (int r = 0; r < logo.rows; r++) {
        for (int c = 0; c < logo.cols; c++) {
            if (c < logo.cell_counts[r])
                hmap[r][c] = text::char_weight_utf8(logo.cells[r][c].data());
            else
                hmap[r][c] = 0.0f;
        }
    }

    for (int r = 0; r < logo.rows; r++) {
        for (int c = 0; c < logo.cols; c++) {
            if (hmap[r][c] <= 0.0f) {
                gnx[r][c] = gny[r][c] = 0.0f;
                gnz[r][c] = 1.0f;
                continue;
            }

            float dhdx = 0.0f, dhdy = 0.0f;

            if (c > 0 && c < logo.cols - 1)
                dhdx = (hmap[r][c + 1] - hmap[r][c - 1]) * 0.5f;
            else if (c == 0)
                dhdx = hmap[r][c + 1] - hmap[r][c];
            else
                dhdx = hmap[r][c] - hmap[r][c - 1];

            if (r > 0 && r < logo.rows - 1)
                dhdy = (hmap[r + 1][c] - hmap[r - 1][c]) * 0.5f;
            else if (r == 0)
                dhdy = hmap[r + 1][c] - hmap[r][c];
            else
                dhdy = hmap[r][c] - hmap[r - 1][c];

            dhdx /= sx;
            dhdy /= sy;

            float nnx = -dhdx;
            float nny =  dhdy;
            float nnz = 1.0f;
            float l   = std::sqrt(nnx * nnx + nny * nny + nnz * nnz);
            gnx[r][c] = nnx / l;
            gny[r][c] = nny / l;
            gnz[r][c] = nnz / l;
        }
    }

    int subdiv = static_cast<int>(size_scale);
    if (subdiv < 1)
        subdiv = 1;

    px.clear();
    py.clear();
    pz.clear();
    nx.clear();
    ny.clear();
    nz.clear();
    pweight.clear();
    pcolor.clear();

    int idx = 0;
    for (int row = 0; row < logo.rows; row++) {
        for (int col = 0; col < logo.cols; col++) {
            float h = hmap[row][col];
            if (h <= 0.0f)
                continue;

            for (int sr = 0; sr < subdiv; sr++) {
                for (int sc = 0; sc < subdiv; sc++) {
                    float frow = row + static_cast<float>(sr) / subdiv;
                    float fcol = col + static_cast<float>(sc) / subdiv;

                    float ih = h;
                    if (sr > 0 || sc > 0) {
                        float fr = static_cast<float>(sr) / subdiv;
                        float fc = static_cast<float>(sc) / subdiv;
                        int nr = row + (sr > 0 ? 1 : 0);
                        int nc = col + (sc > 0 ? 1 : 0);
                        if (nr >= logo.rows)
                            nr = logo.rows - 1;
                        if (nc >= logo.cols)
                            nc = logo.cols - 1;
                        float h00 = hmap[row][col];
                        float h10 = hmap[nr][col];
                        float h01 = hmap[row][nc];
                        float h11 = hmap[nr][nc];
                        ih = h00 * (1 - fr) * (1 - fc) + h10 * fr * (1 - fc) +
                             h01 * (1 - fr) * fc + h11 * fr * fc;
                        if (ih <= 0.0f)
                            continue;
                    }

                    float ox = (fcol - cx) * sx;
                    float oy = (cy - frow) * sy;
                    float zr = ih * zmax;

                    int is_edge = 0;
                    for (int dr = -1; dr <= 1 && !is_edge; dr++) {
                        for (int dc = -1; dc <= 1 && !is_edge; dc++) {
                            if (dr == 0 && dc == 0)
                                continue;
                            int nr = row + dr, nc = col + dc;
                            float nh = 0.0f;
                            if (nr >= 0 && nr < logo.rows && nc >= 0 && nc < logo.cols)
                                nh = hmap[nr][nc];
                            if (nh <= 0.0f)
                                is_edge = 1;
                        }
                    }
                    int layers = (is_edge || ih < 0.15f) ? 2 : z_layers;

                    for (int k = 0; k < layers; k++) {
                        if (idx >= kMaxPoints)
                            break;

                        float t = (static_cast<float>(k) / (layers - 1)) - 0.5f;

                        if (idx >= static_cast<int>(px.size())) {
                            px.push_back(ox);
                            py.push_back(oy);
                            pz.push_back(t * 2.0f * zr);
                            pweight.push_back(ih);
                            pcolor.push_back(logo.cell_color[row][col]);
                        } else {
                            px[idx] = ox;
                            py[idx] = oy;
                            pz[idx] = t * 2.0f * zr;
                            pweight[idx] = ih;
                            pcolor[idx] = logo.cell_color[row][col];
                        }

                        if (k == 0) {
                            if (idx >= static_cast<int>(nx.size())) {
                                nx.push_back( gnx[row][col]);
                                ny.push_back( gny[row][col]);
                                nz.push_back(-gnz[row][col]);
                            } else {
                                nx[idx] =  gnx[row][col];
                                ny[idx] =  gny[row][col];
                                nz[idx] = -gnz[row][col];
                            }
                        } else if (k == layers - 1) {
                            if (idx >= static_cast<int>(nx.size())) {
                                nx.push_back(gnx[row][col]);
                                ny.push_back(gny[row][col]);
                                nz.push_back(gnz[row][col]);
                            } else {
                                nx[idx] = gnx[row][col];
                                ny[idx] = gny[row][col];
                                nz[idx] = gnz[row][col];
                            }
                        } else {
                            float ex = 0.0f, ey = 0.0f;
                            for (int dr = -1; dr <= 1; dr++) {
                                for (int dc = -1; dc <= 1; dc++) {
                                    if (dr == 0 && dc == 0)
                                        continue;
                                    int nr = row + dr, nc = col + dc;
                                    float nh = 0.0f;
                                    if (nr >= 0 && nr < logo.rows && nc >= 0 && nc < logo.cols)
                                        nh = hmap[nr][nc];
                                    if (nh < h) {
                                        ex += static_cast<float>(dc);
                                        ey += static_cast<float>(-dr);
                                    }
                                }
                            }
                            float el = std::sqrt(ex * ex + ey * ey);
                            if (el > 1e-6f) {
                                ex /= el;
                                ey /= el;
                            }
                            float tn   = (static_cast<float>(k) / (layers - 1)) * 2.0f - 1.0f;
                            float side = std::sqrt(1.0f - tn * tn);
                            if (idx >= static_cast<int>(nx.size())) {
                                nx.push_back(ex * side);
                                ny.push_back(ey * side);
                                nz.push_back(tn);
                            } else {
                                nx[idx] = ex * side;
                                ny[idx] = ey * side;
                                nz[idx] = tn;
                            }
                        }
                        idx++;
                    }
                }
            }
        }
    }

    point_count = idx;
    px.resize(idx);
    py.resize(idx);
    pz.resize(idx);
    nx.resize(idx);
    ny.resize(idx);
    nz.resize(idx);
    pweight.resize(idx);
    pcolor.resize(idx);
}

void RenderEngine::compute_threshold() {
    if (point_count == 0)
        return;
    std::vector<float> sorted(pweight.begin(), pweight.end());
    std::sort(sorted.begin(), sorted.end());
    color_threshold = sorted[point_count / 2];
}

} // namespace rfxh::render
