#pragma once

#include "config/cli.hpp"
#include "config/config.hpp"
#include "gather/gather.hpp"
#include "logo/logo.hpp"
#include "logo/colors.hpp"
#include "render/render.hpp"
#include <string>

namespace rfxh::engine {

class App {
public:
    void run(int argc, char** argv);

private:
    // Modules
    config::Config cfg_;
    logo::Logo logo_;
    render::RenderEngine render_;

    // Fetch info lines
    gather::FetchLines fetch_lines_{};
    int fetch_line_count_ = 0;

    // Distro
    std::string distro_;

    // Colors
    std::string color_inner_ = "\033[1;37m";
    std::string color_outer_ = "\033[1;35m";

    // State
    float A_ = 0.0f;
    float B_ = 0.0f;
    float K1_ = 0.0f;
    int render_height_ = 0;
    int fetch_start_ = 0;

    // Helpers
    void load_logo(const config::CliOptions& opts);
    void gather_info(const config::CliOptions& opts);
    void setup_render(const config::CliOptions& opts);
    void animation_loop(const config::CliOptions& opts);
};

} // namespace rfxh::engine
