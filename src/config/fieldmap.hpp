#pragma once

#include "config/fields.hpp"
#include "platform/process.hpp"
#include <cstring>

namespace rfxh::config {

struct FieldEntry {
    const char* name;
    Field id;
};

inline constexpr FieldEntry kFieldMap[] = {
    {"os",       F_OS},
    {"host",     F_HOST},
    {"kernel",   F_KERNEL},
    {"uptime",   F_UPTIME},
    {"packages", F_PACKAGES},
    {"shell",    F_SHELL},
    {"display",  F_DISPLAY},
    {"wm",       F_WM},
    {"theme",    F_THEME},
    {"icons",    F_ICONS},
    {"font",     F_FONT},
    {"terminal", F_TERMINAL},
    {"cpu",      F_CPU},
    {"gpu",      F_GPU},
    {"memory",   F_MEMORY},
    {"swap",     F_SWAP},
    {"disk",     F_DISK},
    {"ip",       F_IP},
    {"battery",  F_BATTERY},
    {"locale",   F_LOCALE},
    {"colors",   F_COLORS},
    {nullptr,    F_COUNT},
};

// Look up a field name, returns F_COUNT if not found.
inline Field lookup_field(const char* name) {
    for (int i = 0; kFieldMap[i].name; i++) {
        if (platform::case_insensitive_cmp(name, kFieldMap[i].name) == 0)
            return kFieldMap[i].id;
    }
    return F_COUNT;
}

} // namespace rfxh::config
