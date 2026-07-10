#pragma once

#include "config/config.hpp"
#include <array>
#include <string>
#include <vector>

namespace rfxh::gather {

inline constexpr int kMaxFetchLines = 32;
inline constexpr int kMaxLineLen = 512;

using FetchLines = std::array<std::array<char, kMaxLineLen>, kMaxFetchLines>;

struct GatherContext {
    config::Config& cfg;
    FetchLines& lines;
    int& line_count;
    std::string& distro;
};

// Core formatting
void add_line(GatherContext& ctx, const char* line);
void add_info(GatherContext& ctx, const char* label, const char* fmt, ...);

// Title
void gather_title(GatherContext& ctx);

// System
void gather_os(GatherContext& ctx);
void gather_host(GatherContext& ctx);
void gather_kernel(GatherContext& ctx);
void gather_uptime(GatherContext& ctx);
void gather_packages(GatherContext& ctx);
void gather_shell(GatherContext& ctx);
void gather_locale(GatherContext& ctx);

// Hardware
void gather_display(GatherContext& ctx);
void gather_cpu(GatherContext& ctx);
void gather_gpu(GatherContext& ctx);
void gather_memory(GatherContext& ctx);
void gather_swap(GatherContext& ctx);
void gather_disk(GatherContext& ctx);
void gather_battery(GatherContext& ctx);

// Desktop
void gather_wm(GatherContext& ctx);
void gather_terminal(GatherContext& ctx);
void gather_ip(GatherContext& ctx);
void gather_theme(GatherContext& ctx);
void gather_icons(GatherContext& ctx);
void gather_font(GatherContext& ctx);

// Colors (handled inline by caller)
void gather_colors(GatherContext& ctx);

} // namespace rfxh::gather
