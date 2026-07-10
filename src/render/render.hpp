#pragma once

#include "common/types.hpp"
#include <string>
#include <vector>

namespace rfxh::logo { struct Logo; }
namespace rfxh::config { struct Config; }
namespace rfxh::gather { using FetchLines = std::array<std::array<char, 512>, 32>; }

namespace rfxh::render {

struct RenderEngine {
    std::vector<float> px, py, pz;
    std::vector<float> nx, ny, nz;
    std::vector<float> pweight;
    std::vector<int>   pcolor;
    int point_count = 0;

    ShadeBuf shade_idx;
    ZBuf     zbuf;
    ColorBuf colorbuf;
    float    color_threshold = 0.5f;

    void build_points(const logo::Logo& logo, float size_scale);
    void compute_threshold();
    void clear_buf(int render_height);
};

// Rasterize a single frame: rotate, project, shade, depth test
void rasterize_frame(RenderEngine& eng, const logo::Logo& logo,
                     float& A, float& B, float speed, bool rotate_x, bool rotate_y,
                     const config::Config& cfg, int render_height);

// Render frame to ANSI output (batch write to stdout)
void render_frame(const RenderEngine& eng, int render_height,
                  const gather::FetchLines& fetch_lines, int fetch_line_count,
                  int fetch_start, const logo::Logo& logo,
                  const std::string& color_inner, const std::string& color_outer,
                  bool use_color);

} // namespace rfxh::render
