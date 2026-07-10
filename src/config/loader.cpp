#include "config/config.hpp"
#include "config/fieldmap.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <strings.h>

namespace rfxh::config {

static void apply_light_preset(Config& cfg, const char* val) {
    if (std::strcmp(val, "top-left") == 0) {
        cfg.light_x = 0.41f; cfg.light_y = 0.82f; cfg.light_z = -0.41f;
    } else if (std::strcmp(val, "top-right") == 0) {
        cfg.light_x = -0.41f; cfg.light_y = 0.82f; cfg.light_z = -0.41f;
    } else if (std::strcmp(val, "top") == 0) {
        cfg.light_x = 0.0f; cfg.light_y = 0.89f; cfg.light_z = -0.45f;
    } else if (std::strcmp(val, "left") == 0) {
        cfg.light_x = 0.82f; cfg.light_y = 0.41f; cfg.light_z = -0.41f;
    } else if (std::strcmp(val, "right") == 0) {
        cfg.light_x = -0.82f; cfg.light_y = 0.41f; cfg.light_z = -0.41f;
    } else if (std::strcmp(val, "front") == 0) {
        cfg.light_x = 0.0f; cfg.light_y = 0.0f; cfg.light_z = -1.0f;
    } else if (std::strcmp(val, "bottom-left") == 0) {
        cfg.light_x = 0.41f; cfg.light_y = -0.82f; cfg.light_z = -0.41f;
    } else if (std::strcmp(val, "bottom-right") == 0) {
        cfg.light_x = -0.41f; cfg.light_y = -0.82f; cfg.light_z = -0.41f;
    }
}

static void apply_label_color(Config& cfg, const char* val) {
    if (std::strcmp(val, "red") == 0)        cfg.label_color = "31";
    else if (std::strcmp(val, "green") == 0)  cfg.label_color = "32";
    else if (std::strcmp(val, "yellow") == 0) cfg.label_color = "33";
    else if (std::strcmp(val, "blue") == 0)   cfg.label_color = "34";
    else if (std::strcmp(val, "magenta") == 0) cfg.label_color = "35";
    else if (std::strcmp(val, "cyan") == 0)   cfg.label_color = "36";
    else if (std::strcmp(val, "white") == 0)  cfg.label_color = "37";
    else cfg.label_color = val;
}

static bool starts_with(const char* line, const char* prefix) {
    return std::strncmp(line, prefix, std::strlen(prefix)) == 0;
}

void load_config(Config& cfg) {
    const char* home = std::getenv("HOME");
    if (!home) return;

    char path[512];
    std::snprintf(path, sizeof(path), "%s/.config/fetch/config", home);
    FILE* fp = std::fopen(path, "r");
    if (!fp) return;

    // Config file exists — reset field state
    cfg.field_enabled = {};
    cfg.field_count = 0;

    char buf[256];
    while (std::fgets(buf, sizeof(buf), fp)) {
        // Trim trailing whitespace/newlines
        int len = static_cast<int>(std::strlen(buf));
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r' || buf[len-1] == ' '))
            buf[--len] = '\0';

        // Skip leading whitespace
        char* line = buf;
        while (*line == ' ' || *line == '\t') line++;

        // Skip comments and empty lines
        if (*line == '#' || *line == '\0') continue;

        // Key=value settings
        if (starts_with(line, "label_color=")) {
            apply_label_color(cfg, line + 12);
        } else if (starts_with(line, "height=")) {
            cfg.config_height = std::atoi(line + 7);
        } else if (starts_with(line, "size=")) {
            cfg.size_scale = static_cast<float>(std::atof(line + 5));
            if (cfg.size_scale < 0.5f) cfg.size_scale = 0.5f;
            if (cfg.size_scale > 5.0f) cfg.size_scale = 5.0f;
        } else if (starts_with(line, "speed=")) {
            cfg.config_speed = static_cast<float>(std::atof(line + 6));
        } else if (starts_with(line, "spin=")) {
            const char* val = line + 5;
            cfg.config_spin_x = (std::strchr(val, 'x') || std::strchr(val, 'X')) ? 1 : 0;
            cfg.config_spin_y = (std::strchr(val, 'y') || std::strchr(val, 'Y')) ? 1 : 0;
        } else if (starts_with(line, "shading=")) {
            cfg.config_shading = line + 8;
        } else if (starts_with(line, "separator=")) {
            cfg.config_separator = line + 10;
        } else if (starts_with(line, "light=")) {
            apply_light_preset(cfg, line + 6);
        } else {
            // Match field name
            Field id = lookup_field(line);
            if (id < F_COUNT && !cfg.field_enabled[id] && cfg.field_count < F_COUNT) {
                cfg.field_enabled[id] = true;
                cfg.field_order[cfg.field_count++] = id;
            }
        }
    }
    std::fclose(fp);
}

} // namespace rfxh::config
