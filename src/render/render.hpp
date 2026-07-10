#pragma once

#include "common/types.hpp"
#include <vector>

namespace rfxh::logo { struct Logo; }

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

} // namespace rfxh::render
