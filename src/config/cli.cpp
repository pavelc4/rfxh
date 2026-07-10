#include "config/cli.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace rfxh::config {

static void print_help() {
    std::printf(
        "Usage: rfxh [options]\n\n"
        "Options:\n"
        "  -l, --logo <name>         Use a logo from fastfetch by name\n"
        "  --rotate-x                Lock rotation to X axis only\n"
        "  --rotate-y                Lock rotation to Y axis only\n"
        "  -s, --speed <float>       Speed multiplier (default 1.0)\n"
        "  --size <float>            Scale the logo (0.5 - 5.0)\n"
        "  --height <n>              Override render height in rows\n"
        "  --no-info                 Just the logo, no system info\n"
        "  --no-color                Disable logo coloring\n"
        "  --frames <n>              Stop after n frames (default 2000)\n"
        "  --infinite                Run forever\n"
        "  --shading-chars <str>     Custom shading ramp (UTF-8)\n"
        "  -h, --help                Show this help\n");
}

CliOptions parse_cli(int argc, char** argv) {
    CliOptions opts;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            print_help();
            std::exit(0);
        } else if ((std::strcmp(argv[i], "--logo") == 0 || std::strcmp(argv[i], "-l") == 0)
                   && i + 1 < argc) {
            opts.logo_name = argv[++i];
        } else if (std::strcmp(argv[i], "--rotate-x") == 0) {
            opts.rotate_x = true;
            opts.rotate_y = false;
        } else if (std::strcmp(argv[i], "--rotate-y") == 0) {
            opts.rotate_x = false;
            opts.rotate_y = true;
        } else if ((std::strcmp(argv[i], "--speed") == 0 || std::strcmp(argv[i], "-s") == 0)
                   && i + 1 < argc) {
            opts.speed = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            opts.size_scale = std::atof(argv[++i]);
            if (opts.size_scale < 0.5f) opts.size_scale = 0.5f;
            if (opts.size_scale > 5.0f) opts.size_scale = 5.0f;
        } else if (std::strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
            opts.config_height = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--no-info") == 0) {
            opts.show_info = false;
        } else if (std::strcmp(argv[i], "--no-color") == 0) {
            opts.use_color = false;
        } else if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            opts.max_frames = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--infinite") == 0) {
            opts.infinite = true;
            opts.max_frames = 0;
        } else if (std::strcmp(argv[i], "--shading-chars") == 0 && i + 1 < argc) {
            opts.shading_chars = argv[++i];
        }
    }

    return opts;
}

} // namespace rfxh::config
