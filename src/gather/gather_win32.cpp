#include "gather/gather.hpp"
#include "platform/sysinfo.hpp"
#include "platform/process.hpp"
#include "config/config.hpp"

#ifdef _WIN32

namespace rfxh::gather {

void add_line(GatherContext& ctx, const char* label, const char* value) {
    if (ctx.cfg.is_refresh_pass && ctx.cfg.field_line[ctx.cfg.current_field] >= 0) {
        int line = ctx.cfg.field_line[ctx.cfg.current_field];
        std::snprintf(ctx.lines[line].data(), 512, "\033[%sm%s:\033[0m %s",
                     ctx.cfg.label_color.c_str(), label, value);
        return;
    }
    int slot = ctx.count;
    if (slot >= 32) return;
    ctx.cfg.field_line[ctx.cfg.current_field] = slot;
    std::snprintf(ctx.lines[slot].data(), 512, "\033[%sm%s:\033[0m %s",
                 ctx.cfg.label_color.c_str(), label, value);
    ctx.count++;
}

void gather_title(GatherContext& ctx) {
    const char* sep = ctx.cfg.config_separator.c_str();
    std::string title;
    auto host = platform::get_host_info();
    auto os = platform::get_os_info();

    title += "\033[1m";
    if (!host.hostname.empty()) title += host.hostname;
    title += "\033[0m";
    for (int i = 0; i < 3; i++) title += " ";
    for (int i = 0; i < (int)title.size() - 12; i++) title += *sep;

    int slot = ctx.count;
    if (slot < 32) {
        std::snprintf(ctx.lines[slot].data(), 512, "%s", title.c_str());
        ctx.count++;
    }
}

void gather_colors(GatherContext& ctx) {
    int slot = ctx.count;
    if (slot >= 32) return;
    char* buf = ctx.lines[slot].data();
    int off = 0;
    for (int i = 40; i <= 47; i++)
        off += std::snprintf(buf + off, 512 - off, "\033[%dm   ", i);
    for (int i = 100; i <= 107; i++)
        off += std::snprintf(buf + off, 512 - off, "\033[%dm   ", i);
    off += std::snprintf(buf + off, 512 - off, "\033[0m");
    ctx.count++;
}

void gather_os(GatherContext& ctx) {
    auto info = platform::get_os_info();
    add_line(ctx, "OS", info.name.c_str());
}

void gather_host(GatherContext& ctx) {
    auto info = platform::get_host_info();
    add_line(ctx, "Host", info.product.c_str());
}

void gather_kernel(GatherContext& ctx) {
    auto ver = platform::get_kernel_version();
    add_line(ctx, "Kernel", ver.c_str());
}

void gather_uptime(GatherContext& ctx) {
    auto uptime = platform::get_uptime();
    add_line(ctx, "Uptime", uptime.c_str());
}

void gather_packages(GatherContext& ctx) {
    int count = platform::get_package_count();
    add_line(ctx, "Packages", std::to_string(count).c_str());
}

void gather_shell(GatherContext& ctx) {
    auto shell = platform::get_shell_info();
    add_line(ctx, "Shell", shell.c_str());
}

void gather_display(GatherContext& ctx) {
    auto info = platform::get_display_info();
    std::string val = info.name + " " + info.resolution;
    add_line(ctx, "Display", val.c_str());
}

void gather_wm(GatherContext& ctx) {
    auto wm = platform::get_wm_info();
    add_line(ctx, "WM", wm.c_str());
}

void gather_theme(GatherContext& ctx) {
    auto theme = platform::get_theme_info();
    add_line(ctx, "Theme", theme.c_str());
}

void gather_icons(GatherContext& ctx) {
    auto icons = platform::get_icons_info();
    add_line(ctx, "Icons", icons.c_str());
}

void gather_font(GatherContext& ctx) {
    auto font = platform::get_font_info();
    add_line(ctx, "Font", font.c_str());
}

void gather_terminal(GatherContext& ctx) {
    auto term = platform::get_terminal_info();
    add_line(ctx, "Terminal", term.c_str());
}

void gather_cpu(GatherContext& ctx) {
    auto info = platform::get_cpu_info();
    std::string val = info.model + " (" + std::to_string(info.cores) + ")";
    if (info.freq_mhz > 0)
        val += " @ " + std::to_string(static_cast<int>(info.freq_mhz)) + " MHz";
    add_line(ctx, "CPU", val.c_str());
}

void gather_gpu(GatherContext& ctx) {
    auto info = platform::get_gpu_info();
    add_line(ctx, "GPU", info.model.c_str());
}

void gather_memory(GatherContext& ctx) {
    auto info = platform::get_memory_info();
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.0f MB / %.0f MB",
                 info.used_mb, info.total_mb);
    add_line(ctx, "Memory", buf);
}

void gather_swap(GatherContext& ctx) {
    auto info = platform::get_memory_info();
    if (info.swap_total_mb <= 0) {
        add_line(ctx, "Swap", "N/A");
        return;
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.0f MB / %.0f MB",
                 info.swap_used_mb, info.swap_total_mb);
    add_line(ctx, "Swap", buf);
}

void gather_disk(GatherContext& ctx) {
    auto info = platform::get_disk_info();
    char buf[128];
    std::snprintf(buf, sizeof(buf), "%.1f GB / %.1f GB (%s)",
                 info.used_gb, info.total_gb, info.mount.c_str());
    add_line(ctx, "Disk", buf);
}

void gather_ip(GatherContext& ctx) {
    auto ip = platform::get_ip_address();
    add_line(ctx, "IP", ip.c_str());
}

void gather_battery(GatherContext& ctx) {
    auto info = platform::get_battery_info();
    if (info.capacity < 0) {
        add_line(ctx, "Battery", "N/A");
        return;
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d%% [%s]", info.capacity, info.status.c_str());
    add_line(ctx, "Battery", buf);
}

void gather_locale(GatherContext& ctx) {
    auto locale = platform::get_locale();
    add_line(ctx, "Locale", locale.c_str());
}

} // namespace rfxh::gather

#endif // _WIN32
