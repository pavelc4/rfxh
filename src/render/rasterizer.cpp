#include "render/render.hpp"
#include "render/constants.hpp"
#include "logo/logo.hpp"
#include "text/shading.hpp"
#include "config/config.hpp"
#include "common/types.hpp"

#include <cmath>

namespace rfxh::render {

void rasterize_frame(RenderEngine& eng, const logo::Logo& logo,
                     float& A, float& B, float speed, bool rotate_x, bool rotate_y,
                     const config::Config& cfg, int render_height) {

    eng.clear_buf(render_height);

    A += rotate_x ? 0.04f * speed : 0.0f;
    B += rotate_y ? 0.06f * speed : 0.0f;

    float cA = std::cos(A), sA = std::sin(A);
    float cB = std::cos(B), sB = std::sin(B);

    const float lx = cfg.light_x, ly = cfg.light_y, lz = cfg.light_z;

    const float hx0 = lx, hy0 = ly, hz0 = lz - 1.0f;
    const float hl0 = std::sqrt(hx0 * hx0 + hy0 * hy0 + hz0 * hz0);
    const float hlx = hx0 / hl0, hly = hy0 / hl0, hlz = hz0 / hl0;

    const float K1 = 37.0f * render_height / 36.0f;
    const float K2 = 5.5f;
    const float k1x2 = K1 * 2.0f;
    const float half_aw = static_cast<float>(kFrameWidth) * 0.5f;
    const int aw = kFrameWidth;
    const int smax = text::shading_count() - 1;

    const float y_center = render_height * 0.5f;

    for (int i = 0; i < eng.point_count; i++) {
        float px_i = eng.px[i], py_i = eng.py[i], pz_i = eng.pz[i];
        float nx_i = eng.nx[i], ny_i = eng.ny[i], nz_i = eng.nz[i];

        float y1 = py_i * cA - pz_i * sA;
        float z1 = py_i * sA + pz_i * cA;
        float x2 = px_i * cB + z1 * sB;
        float z2 = -px_i * sB + z1 * cB;
        float y2 = y1;

        float ny1 = ny_i * cA - nz_i * sA;
        float nz1 = ny_i * sA + nz_i * cA;
        float nx2 = nx_i * cB + nz1 * sB;
        float nz2 = -nx_i * sB + nz1 * cB;
        float ny2 = ny1;

        float zc = z2 + K2;
        if (zc < 0.1f)
            continue;
        float ooz = 1.0f / zc;
        int xs = static_cast<int>(half_aw + k1x2 * x2 * ooz);
        int ys = static_cast<int>(y_center - K1 * y2 * ooz);
        if (xs < 0 || xs >= aw || ys < 0 || ys >= render_height)
            continue;

        if (ooz > eng.zbuf[ys][xs]) {
            float diff = nx2 * lx + ny2 * ly + nz2 * lz;
            if (diff < 0.0f)
                diff = 0.0f;

            float spec_dot = nx2 * hlx + ny2 * hly + nz2 * hlz;
            if (spec_dot < 0.0f)
                spec_dot = 0.0f;
            float spec = spec_dot * spec_dot;
            spec = spec * spec;
            spec = spec * spec;

            float L = 0.08f + 0.62f * diff + 0.30f * spec;
            if (L > 1.0f)
                L = 1.0f;

            eng.zbuf[ys][xs] = ooz;
            int ci = static_cast<int>(L * smax);
            if (ci < 0) ci = 0;
            if (ci > smax) ci = smax;
            eng.shade_idx[ys][xs] = ci;
            eng.colorbuf[ys][xs] = logo.has_ansi
                                       ? eng.pcolor[i]
                                       : ((eng.pweight[i] >= eng.color_threshold) ? 1 : 0);
        }
    }
}

} // namespace rfxh::render
