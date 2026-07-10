#pragma once

#include "common/types.hpp"
#include <array>
#include <string>

namespace rfxh::logo {

struct Logo {
    std::array<LineBuf, kMaxLogoRows> data{};
    std::array<std::array<Codepoint, kMaxLogoCols>, kMaxLogoRows> cells{};
    std::array<std::array<int, kMaxLogoCols>, kMaxLogoRows> cell_color{};
    std::array<int, kMaxLogoRows> cell_counts{};
    int rows = 0;
    int cols = 0;
    bool has_ansi = false;
    std::string file_distro;
};

// Load logo from ~/.config/fetch/logo.txt
bool load_logo_file(Logo& logo);

// Load logo from built-in library (case-insensitive name match)
bool load_logo_library(Logo& logo, const char* name);

// Load logo via fastfetch (colored first, then plain fallback)
bool load_logo_fastfetch(Logo& logo, const char* name);

// Load built-in default Gentoo logo
void load_default_logo(Logo& logo);

// Process raw logo data into parsed codepoint cells
void process_logo(Logo& logo);

} // namespace rfxh::logo
