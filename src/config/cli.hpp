#pragma once

#include <string>

namespace rfxh::config {

struct CliOptions {
    std::string logo_name;
    std::string shading_chars = ".,-~:;=!*#$@";
    float speed       = 1.0f;
    float size_scale  = 1.0f;
    int config_height = 0;
    int max_frames    = 2000;
    bool rotate_x     = true;
    bool rotate_y     = true;
    bool show_info    = true;
    bool use_color    = true;
    bool infinite     = false;
};

CliOptions parse_cli(int argc, char** argv);

} // namespace rfxh::config
