#include "config/config.hpp"

namespace rfxh::config {

void config_defaults(Config& cfg) {
    constexpr int defaults[] = {
        F_OS, F_HOST, F_KERNEL, F_UPTIME, F_PACKAGES, F_SHELL, F_DISPLAY,
        F_WM, F_THEME, F_ICONS, F_FONT, F_TERMINAL, F_CPU, F_GPU,
        F_MEMORY, F_SWAP, F_DISK, F_IP, F_BATTERY, F_LOCALE, F_COLORS
    };
    cfg.field_count = sizeof(defaults) / sizeof(defaults[0]);
    for (int i = 0; i < cfg.field_count; i++) {
        cfg.field_order[i] = defaults[i];
        cfg.field_enabled[defaults[i]] = true;
    }
}

} // namespace rfxh::config
