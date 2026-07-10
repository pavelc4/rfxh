#include "engine/app.hpp"
#include "config/fields.hpp"
#include "logo/detect.hpp"
#include "logo/detect.hpp"
#include "render/constants.hpp"
#include "terminal/terminal.hpp"
#include "text/shading.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <poll.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

namespace rfxh::engine {

using gather_fn = void (*)(gather::GatherContext&);

static gather_fn gather_table[config::F_COUNT] = {};

static void init_gather_table() {
    gather_table[config::F_OS]       = +[](gather::GatherContext& c) { gather::gather_os(c); };
    gather_table[config::F_HOST]     = +[](gather::GatherContext& c) { gather::gather_host(c); };
    gather_table[config::F_KERNEL]   = +[](gather::GatherContext& c) { gather::gather_kernel(c); };
    gather_table[config::F_UPTIME]   = +[](gather::GatherContext& c) { gather::gather_uptime(c); };
    gather_table[config::F_PACKAGES] = +[](gather::GatherContext& c) { gather::gather_packages(c); };
    gather_table[config::F_SHELL]    = +[](gather::GatherContext& c) { gather::gather_shell(c); };
    gather_table[config::F_DISPLAY]  = +[](gather::GatherContext& c) { gather::gather_display(c); };
    gather_table[config::F_WM]       = +[](gather::GatherContext& c) { gather::gather_wm(c); };
    gather_table[config::F_THEME]    = +[](gather::GatherContext& c) { gather::gather_theme(c); };
    gather_table[config::F_ICONS]    = +[](gather::GatherContext& c) { gather::gather_icons(c); };
    gather_table[config::F_FONT]     = +[](gather::GatherContext& c) { gather::gather_font(c); };
    gather_table[config::F_TERMINAL] = +[](gather::GatherContext& c) { gather::gather_terminal(c); };
    gather_table[config::F_CPU]      = +[](gather::GatherContext& c) { gather::gather_cpu(c); };
    gather_table[config::F_GPU]      = +[](gather::GatherContext& c) { gather::gather_gpu(c); };
    gather_table[config::F_MEMORY]   = +[](gather::GatherContext& c) { gather::gather_memory(c); };
    gather_table[config::F_SWAP]     = +[](gather::GatherContext& c) { gather::gather_swap(c); };
    gather_table[config::F_DISK]     = +[](gather::GatherContext& c) { gather::gather_disk(c); };
    gather_table[config::F_IP]       = +[](gather::GatherContext& c) { gather::gather_ip(c); };
    gather_table[config::F_BATTERY]  = +[](gather::GatherContext& c) { gather::gather_battery(c); };
    gather_table[config::F_LOCALE]   = +[](gather::GatherContext& c) { gather::gather_locale(c); };
}

void App::run(int argc, char** argv) {
    init_gather_table();

    auto opts = config::parse_cli(argc, argv);

    // Parse shading ramp
    text::parse_shading(opts.shading_chars.c_str());

    // Config
    config::config_defaults(cfg_);
    config::load_config(cfg_);

    // Config overrides (CLI flags take priority)
    if (!cfg_.config_shading.empty())
        text::parse_shading(cfg_.config_shading.c_str());
    if (cfg_.config_speed > 0 && opts.speed == 1.0f)
        opts.speed = cfg_.config_speed;
    if (cfg_.config_spin_x >= 0 && opts.rotate_x && opts.rotate_y) {
        opts.rotate_x = cfg_.config_spin_x;
        opts.rotate_y = cfg_.config_spin_y;
    }

    load_logo(opts);

    // Process logo into codepoint cells
    logo::process_logo(logo_);

    // Set distro colors
    if (!distro_.empty()) {
        auto colors = logo::get_distro_colors(distro_.c_str());
        color_inner_ = colors.inner;
        color_outer_ = colors.outer;
    }

    // Gather system info
    gather_info(opts);

    // Setup render dimensions
    setup_render(opts);

    // Build point cloud
    render_.build_points(logo_, opts.size_scale);
    render_.compute_threshold();

    // Animation loop
    animation_loop(opts);
}

void App::load_logo(const config::CliOptions& opts) {
    if (!opts.logo_name.empty()) {
        if (!logo::load_logo_fastfetch(logo_, opts.logo_name.c_str()))
            logo::load_default_logo(logo_);
        distro_ = opts.logo_name;
        return;
    }

    // Try custom logo.txt
    bool has_custom = logo::load_logo_file(logo_);
    if (!logo_.file_distro.empty())
        distro_ = logo_.file_distro;
    else {
        auto info = logo::detect_distro();
        distro_ = info.id;
    }

    // Try fastfetch if no custom logo
    bool got_logo = has_custom;
    if (!got_logo && !distro_.empty()) {
        got_logo = logo::load_logo_fastfetch(logo_, distro_.c_str());
        if (!got_logo) {
            // Try ID_LIKE fallback
            auto info = logo::detect_distro();
            // Tokenize id_like by space
            std::string likes = info.id_like;
            std::size_t pos = 0;
            while (!got_logo && (pos = likes.find(' ')) != std::string::npos) {
                std::string tok = likes.substr(0, pos);
                likes = likes.substr(pos + 1);
                if (logo::load_logo_fastfetch(logo_, tok.c_str())) {
                    got_logo = true;
                    distro_ = tok;
                }
            }
            if (!got_logo && !likes.empty()) {
                if (logo::load_logo_fastfetch(logo_, likes.c_str())) {
                    distro_ = likes;
                }
            }
        }
    }
    if (!got_logo && logo_.rows == 0) {
        logo::load_default_logo(logo_);
        if (!distro_.empty() && distro_ != "gentoo")
            std::fprintf(stderr, "rfxh: couldn't load %s logo (is fastfetch installed?). "
                         "using built-in gentoo logo.\n", distro_.c_str());
    }
}

void App::gather_info(const config::CliOptions& opts) {
    if (!opts.show_info) return;

    for (int i = 0; i < config::F_COUNT; i++)
        cfg_.field_line[i] = -1;

    gather::GatherContext ctx{cfg_, fetch_lines_, fetch_line_count_, distro_};

    gather::gather_title(ctx);

    for (int i = 0; i < cfg_.field_count; i++) {
        int id = cfg_.field_order[i];
        if (id == config::F_COLORS) {
            gather::gather_colors(ctx);
        } else if (id < config::F_COUNT && gather_table[id]) {
            cfg_.current_field = id;
            gather_table[id](ctx);
        }
    }
    cfg_.current_field = -1;
}

void App::setup_render(const config::CliOptions& opts) {
    // Render height
    if (cfg_.config_height > 0) {
        render_height_ = cfg_.config_height;
    } else if (opts.show_info && fetch_line_count_ > 0) {
        int info_height = fetch_line_count_ + 2;
        render_height_ = info_height > 36 ? info_height : 36;
    } else {
        render_height_ = 36;
    }

    // Apply size scale
    render_height_ = static_cast<int>(render_height_ * opts.size_scale);
    if (render_height_ < 20) render_height_ = 20;
    if (render_height_ > render::kFrameHeight) render_height_ = render::kFrameHeight;

    // Cap to terminal height
    int term_rows = terminal::get_term_rows();
    if (term_rows > 1) term_rows--;
    if (term_rows > 0 && render_height_ > term_rows)
        render_height_ = term_rows;

    K1_ = 37.0f * render_height_ / 36.0f;
    fetch_start_ = opts.show_info ? 1 : 0;
}

void App::animation_loop(const config::CliOptions& opts) {
    terminal::install_signal_handlers();
    terminal::RawModeGuard guard;

    std::printf("\033[?25l\033[2J");
    std::fflush(stdout);

    for (int frame = 0; opts.max_frames == 0 || frame < opts.max_frames; frame++) {
        // Check for keypress
        struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
        if (poll(&pfd, 1, 0) > 0)
            break;

        // Handle terminal resize
        if (terminal::consume_resize_flag()) {
            int new_rows = terminal::get_term_rows();
            if (new_rows > 1) new_rows--;
            if (new_rows > 0 && new_rows != render_height_) {
                render_height_ = new_rows;
                if (render_height_ > render::kFrameHeight)
                    render_height_ = render::kFrameHeight;
                K1_ = 37.0f * render_height_ / 36.0f;
                std::printf("\033[2J");
                std::fflush(stdout);
            }
        }

        // Refresh dynamic fields every 20 frames
        if (opts.show_info && frame > 0 && frame % 20 == 0) {
            cfg_.is_refresh_pass = true;
            if (cfg_.field_line[config::F_UPTIME] >= 0) {
                cfg_.current_field = config::F_UPTIME;
                gather::GatherContext ctx{cfg_, fetch_lines_, fetch_line_count_, distro_};
                gather::gather_uptime(ctx);
            }
            if (cfg_.field_line[config::F_MEMORY] >= 0) {
                cfg_.current_field = config::F_MEMORY;
                gather::GatherContext ctx{cfg_, fetch_lines_, fetch_line_count_, distro_};
                gather::gather_memory(ctx);
            }
            if (cfg_.field_line[config::F_SWAP] >= 0) {
                cfg_.current_field = config::F_SWAP;
                gather::GatherContext ctx{cfg_, fetch_lines_, fetch_line_count_, distro_};
                gather::gather_swap(ctx);
            }
            cfg_.current_field = -1;
            cfg_.is_refresh_pass = false;
        }

        // Rasterize frame
        render::rasterize_frame(render_, logo_, A_, B_, opts.speed, opts.rotate_x, opts.rotate_y,
                                cfg_, render_height_);

        // Render to stdout
        render::render_frame(render_, render_height_, fetch_lines_, fetch_line_count_,
                             fetch_start_, logo_, color_inner_, color_outer_, opts.use_color);

        // Advance rotation
        A_ += opts.rotate_x ? 0.04f * opts.speed : 0.0f;
        B_ += opts.rotate_y ? 0.06f * opts.speed : 0.0f;

        usleep(50000);
    }

    std::printf("\033[?25h");
    std::fflush(stdout);
}

} // namespace rfxh::engine
