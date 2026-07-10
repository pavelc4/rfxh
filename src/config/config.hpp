#pragma once

#include "config/fields.hpp"
#include <array>
#include <string>

namespace rfxh::config {

struct Config {
    std::array<bool, F_COUNT> field_enabled{};
    std::array<int, F_COUNT> field_order{};
    std::array<int, F_COUNT> field_line{};   // which fetch_lines slot each field occupies
    int field_count = 0;
    int current_field = -1;
    bool is_refresh_pass = false;

    int config_height = 0;
    float size_scale = 1.0f;
    float config_speed = 0.0f;
    int config_spin_x = -1;
    int config_spin_y = -1;
    std::string label_color = "35";
    std::string config_shading;
    std::string config_separator = "-";
    float light_x = 0.4082f;
    float light_y = 0.8165f;
    float light_z = -0.4082f;
};

void config_defaults(Config& cfg);
void load_config(Config& cfg);

} // namespace rfxh::config
