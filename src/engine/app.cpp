#include "engine/app.hpp"
#include "config/fields.hpp"
#include "logo/detect.hpp"
#include "platform/terminal.hpp"
#include "render/constants.hpp"
#include "terminal/terminal.hpp"
#include "text/shading.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

namespace rfxh::engine {

// Visible width of a fetch line (strip ANSI CSI ... letter)
static int visible_len(const char* s) {
    int n = 0;
    for (const char* p = s; *p; p++) {
        if (*p == '\033' && *(p + 1) == '[') {
            p += 2;
            while (*p && (*p < '@' || *p > '~')) p++;
            if (!*p) break;
        } else {
            n++;
        }
    }
    return n;
}

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
        // Try library first, then fastfetch
        if (!logo::load_logo_library(logo_, opts.logo_name.c_str()) &&
            !logo::load_logo_fastfetch(logo_, opts.logo_name.c_str()))
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

    // Try library or fastfetch if no custom logo
    bool got_logo = has_custom;
    if (!got_logo && !distro_.empty()) {
        got_logo = logo::load_logo_library(logo_, distro_.c_str()) ||
                   logo::load_logo_fastfetch(logo_, distro_.c_str());
        if (!got_logo) {
            // Try ID_LIKE fallback
            auto info = logo::detect_distro();
            std::string likes = info.id_like;
            std::size_t pos = 0;
            while (!got_logo && (pos = likes.find(' ')) != std::string::npos) {
                std::string tok = likes.substr(0, pos);
                likes = likes.substr(pos + 1);
                if (logo::load_logo_library(logo_, tok.c_str()) ||
                    logo::load_logo_fastfetch(logo_, tok.c_str())) {
                    got_logo = true;
                    distro_ = tok;
                }
            }
            if (!got_logo && !likes.empty()) {
                if (logo::load_logo_library(logo_, likes.c_str()) ||
                    logo::load_logo_fastfetch(logo_, likes.c_str())) {
                    distro_ = likes;
                }
            }
        }
    }
    if (!got_logo && logo_.rows == 0) {
        logo::load_default_logo(logo_);
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

void App::compute_sizes(const config::CliOptions& opts) {
    // Height (as before)
    if (cfg_.config_height > 0) {
        render_height_ = cfg_.config_height;
    } else if (opts.show_info && fetch_line_count_ > 0) {
        int info_height = fetch_line_count_ + 2;
        render_height_ = info_height > 36 ? info_height : 36;
    } else {
        render_height_ = 36;
    }
    render_height_ = static_cast<int>(render_height_ * opts.size_scale);
    if (render_height_ < 20) render_height_ = 20;
    if (render_height_ > render::kFrameHeight) render_height_ = render::kFrameHeight;
    int term_rows = platform::terminal_rows();
    if (term_rows <= 0) term_rows = terminal::get_term_rows();
    if (term_rows > 1) term_rows--;
    if (term_rows > 0 && render_height_ > term_rows)
        render_height_ = term_rows;

    // Width: fit logo + info side-by-side, else shrink logo, else drop info
    render_width_ = render::kFrameWidth;
    int term_cols = platform::terminal_cols();
    if (term_cols > 0) {
        if (!opts.show_info) {
            render_width_ = std::min(render::kFrameWidth, term_cols - 1);
        } else {
            int info_w = 0;
            for (int i = 0; i < fetch_line_count_; i++)
                info_w = std::max(info_w, visible_len(fetch_lines_[i].data()));
            int avail = term_cols - info_w - render::kGap - 1;
            if (avail >= render::kFrameWidth) {
                render_width_ = render::kFrameWidth;
            } else if (avail >= 20) {
                render_width_ = avail;
            } else {
                // ponytail: too narrow -> logo only, info would wrap
                render_width_ = std::min(render::kFrameWidth, term_cols - 1);
            }
        }
        if (render_width_ < 10) render_width_ = 10;
        if (render_width_ > render::kFrameWidth) render_width_ = render::kFrameWidth;
    }

    K1_ = 37.0f * render_height_ / 36.0f;
    fetch_start_ = opts.show_info ? 1 : 0;
}

void App::setup_render(const config::CliOptions& opts) {
    compute_sizes(opts);
}

// Flat key → action table. Returns: 1 quit, 2 space, 3 reset, 0 handled-as-rotate/none.
static int plain_key(unsigned char c, float& dA, float& dB) {
    switch (c | 0x20) { // fold upper→lower, non-letters unaffected for our cases
        case 'q': return 1;
        case ' ': return 2;
        case 'r': return 3;
        case 'w': case 'k': dA -= 0.12f; return 0;
        case 's': case 'j': dA += 0.12f; return 0;
        case 'a': case 'h': dB -= 0.12f; return 0;
        case 'd': case 'l': dB += 0.12f; return 0;
        case '+': case '=': return 4; // zoom in
        case '-': return 5;           // zoom out
        default: return 0;
    }
}

static bool arrow_key(char f, float& dA, float& dB) {
    switch (f) {
        case 'A': dA -= 0.15f; return true;
        case 'B': dA += 0.15f; return true;
        case 'C': dB += 0.15f; return true;
        case 'D': dB -= 0.15f; return true;
        default: return false;
    }
}

// End index of SGR mouse seq starting at '<' (i points at ESC), -1 if incomplete.
static int sgr_end(const char* b, int n, int i) {
    if (i + 3 >= n || b[i + 1] != '[' || b[i + 2] != '<') return -2; // not SGR
    int j = i + 3;
    while (j < n && b[j] != 'M' && b[j] != 'm') j++;
    return j >= n ? -1 : j;
}

// Drain pending input, apply drag/arrows. True = quit.
bool App::handle_input(const config::CliOptions& opts) {
    (void)opts;
    int mdx = 0, mdy = 0;
    if (platform::poll_mouse_drag(mdx, mdy)) {
        B_ += mdx * 0.08f;
        A_ += mdy * 0.08f;
    }
    if (int w = platform::poll_mouse_wheel()) {
        while (w > 0) { zoom_ = std::min(zoom_ * 1.08f, 3.0f); w--; }
        while (w < 0) { zoom_ = std::max(zoom_ / 1.08f, 0.4f); w++; }
        idle_frames_ = 0;
    }

    char buf[256];
    int n = 0;
    while (platform::keypress_available() && n < 255) {
        int c = platform::keypress_read();
        if (c <= 0) break;
        buf[n++] = static_cast<char>(c);
    }
    if (n == 0) return false;

    float dA = 0, dB = 0;
    int spaces = 0;
    bool reset = false, quit = false;

    for (int i = 0; i < n && !quit; i++) {
        unsigned char c = buf[i];
        if (c == 0x03) { quit = true; continue; } // Ctrl-C
        if (c != 0x1b) {
            int act = plain_key(c, dA, dB);
            quit = act == 1;
            spaces += act == 2;
            reset = reset || act == 3;
            if (act == 4) { zoom_ = std::min(zoom_ * 1.1f, 3.0f); idle_frames_ = 0; }
            if (act == 5) { zoom_ = std::max(zoom_ / 1.1f, 0.4f); idle_frames_ = 0; }
            continue;
        }
        if (i + 1 >= n) { quit = true; continue; } // lone ESC
        if (buf[i + 1] == 'O' && i + 2 < n) { arrow_key(buf[i + 2], dA, dB); i += 2; continue; }
        if (buf[i + 1] != '[') { i++; continue; }
        int j = sgr_end(buf, n, i);
        if (j == -2) { // plain CSI, skip params
            j = i + 2;
            while (j < n && (buf[j] == ';' || (buf[j] >= '0' && buf[j] <= '9'))) j++;
            if (j < n) arrow_key(buf[j], dA, dB);
            i = j;
            continue;
        }
        if (j < 0) break; // incomplete SGR, drop
        int btn = -1, x = -1, y = -1;
        std::sscanf(buf + i + 3, "%d;%d;%d", &btn, &x, &y);
        if (buf[j] == 'm') dragging_ = false;
        else if (btn == 64) { zoom_ = std::min(zoom_ * 1.08f, 3.0f); idle_frames_ = 0; }
        else if (btn == 65) { zoom_ = std::max(zoom_ / 1.08f, 0.4f); idle_frames_ = 0; }
        else if (x > 0 && y > 0) {
            if (!dragging_) { dragging_ = true; last_mx_ = x; last_my_ = y; }
            else { dB += (x - last_mx_) * 0.08f; dA += (y - last_my_) * 0.08f; last_mx_ = x; last_my_ = y; }
        }
        i = j;
    }
    A_ += dA;
    B_ += dB;
    if (dA != 0.0f || dB != 0.0f || dragging_) idle_frames_ = 0; // manual input: freeze auto-rotate
    if (reset) { A_ = B_ = 0.0f; zoom_ = 1.0f; }
    if (spaces % 2 == 1) paused_ = !paused_;
    return quit;
}

void App::animation_loop(const config::CliOptions& opts) {
    terminal::install_signal_handlers();
    terminal::RawModeGuard guard;

    platform::cursor_hide();
    platform::screen_clear();

    int last_rows = platform::terminal_rows();
    int last_cols = platform::terminal_cols();
    auto last_t = std::chrono::steady_clock::now();
    float fps = 0.0f;

    for (int frame = 0; opts.max_frames == 0 || frame < opts.max_frames; frame++) {
        if (handle_input(opts))
            break;

        // Resize: signal (POSIX) or polled size change (Win32 + fallback)
        bool resized = platform::consume_resize();
        int cur_rows = platform::terminal_rows();
        int cur_cols = platform::terminal_cols();
        if (!resized && ((cur_rows > 0 && cur_rows != last_rows) ||
                         (cur_cols > 0 && cur_cols != last_cols)))
            resized = true;
        if (resized) {
            int h_before = render_height_, w_before = render_width_;
            compute_sizes(opts);
            last_rows = cur_rows;
            last_cols = cur_cols;
            if (render_height_ != h_before || render_width_ != w_before)
                platform::screen_clear();
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

        // Rasterize frame (paused = freeze auto-rotation, drag/keys still work)
        // + idle freeze: manual input stops auto-rotate, resumes after ~3s (60 frames)
        if (idle_frames_ < 1000000) idle_frames_++;
        bool auto_on = !paused_ && idle_frames_ > 60;
        float eff_speed = auto_on ? opts.speed : 0.0f;
        bool eff_rx = auto_on && opts.rotate_x;
        bool eff_ry = auto_on && opts.rotate_y;
        render::rasterize_frame(render_, logo_, A_, B_, eff_speed, eff_rx, eff_ry,
                                cfg_, render_height_, render_width_, zoom_);

        // Hide info if it no longer fits (avoid line-wrap breaking the anim)
        int info_count = fetch_line_count_;
        if (opts.show_info && info_count > 0) {
            int info_w = 0;
            for (int i = 0; i < fetch_line_count_; i++)
                info_w = std::max(info_w, visible_len(fetch_lines_[i].data()));
            int cols = platform::terminal_cols();
            if (cols > 0 && render_width_ + render::kGap + info_w + 1 > cols)
                info_count = 0;
        }

        // Render to stdout
        render::render_frame(render_, render_height_, render_width_, fetch_lines_, info_count,
                             fetch_start_, logo_, color_inner_, color_outer_, opts.use_color);

        if (opts.show_fps) {
            auto now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - last_t).count();
            last_t = now;
            if (dt > 0.0001f) fps = fps * 0.9f + (1.0f / dt) * 0.1f;
            std::printf("\033[K%.1f FPS\n", fps);
            std::fflush(stdout);
        }

        if (!opts.unlimited) platform::sleep_ms(50);
    }

    platform::cursor_show();
}

} // namespace rfxh::engine
