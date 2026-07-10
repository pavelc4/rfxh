#include "platform/sysinfo.hpp"
#include "platform/process.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/utsname.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

namespace rfxh::platform {

// Helper: read first line from file
static std::string read_first_line(const char* path) {
    FILE* fp = std::fopen(path, "r");
    if (!fp) return "";
    char buf[512];
    if (std::fgets(buf, sizeof(buf), fp)) {
        // Trim newline
        char* p = buf;
        while (*p && *p != '\n' && *p != '\r') p++;
        *p = '\0';
        std::fclose(fp);
        return buf;
    }
    std::fclose(fp);
    return "";
}

// Helper: read file value for a key (KEY=VALUE format)
static std::string read_os_release_key(const char* key) {
    FILE* fp = std::fopen("/etc/os-release", "r");
    if (!fp) return "";
    char buf[512];
    size_t klen = std::strlen(key);
    while (std::fgets(buf, sizeof(buf), fp)) {
        if (std::strncmp(buf, key, klen) == 0 && buf[klen] == '=') {
            const char* val = buf + klen + 1;
            // Strip quotes
            if (*val == '"' || *val == '\'') {
                char q = *val++;
                const char* end = val;
                while (*end && *end != q) end++;
                std::string result(val, end);
                std::fclose(fp);
                return result;
            } else {
                const char* end = val;
                while (*end && *end != '\n' && *end != '\r') end++;
                std::string result(val, end);
                std::fclose(fp);
                return result;
            }
        }
    }
    std::fclose(fp);
    return "";
}

// Helper: count subdirectories
static int count_subdirs(const char* path) {
    DIR* d = opendir(path);
    if (!d) return 0;
    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        if (entry->d_name[0] == '.') continue;
        count++;
    }
    closedir(d);
    return count;
}

// Helper: count lines matching prefix
static int count_file_lines(const char* path, const char* prefix) {
    FILE* fp = std::fopen(path, "r");
    if (!fp) return 0;
    int count = 0;
    char buf[512];
    size_t plen = std::strlen(prefix);
    while (std::fgets(buf, sizeof(buf), fp)) {
        if (std::strncmp(buf, prefix, plen) == 0) count++;
    }
    std::fclose(fp);
    return count;
}

OsInfo get_os_info() {
    OsInfo info;
    info.name = read_os_release_key("NAME");
    info.version = read_os_release_key("VERSION_ID");
    info.id = read_os_release_key("ID");
    info.id_like = read_os_release_key("ID_LIKE");

    if (info.name.empty()) {
        info.name = "Linux";
        info.id = "linux";
    }
    return info;
}

HostInfo get_host_info() {
    HostInfo info;

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
        info.hostname = hostname;

    // Try ARM device tree first
    auto model = read_first_line("/proc/device-tree/model");
    if (!model.empty()) {
        info.product = model;
        return info;
    }

    // Try DMI
    auto dmi = read_first_line("/sys/class/dmi/id/product_name");
    if (!dmi.empty()) {
        info.product = dmi;
        return info;
    }

    // Fallback: fastfetch
    auto ff = run_command("fastfetch --format '{system.productName}' --pipe false 2>/dev/null");
    if (!ff.empty())
        info.product = ff;

    return info;
}

std::string get_kernel_version() {
    struct utsname u;
    if (uname(&u) == 0)
        return std::string(u.release) + " " + u.machine;
    return "unknown";
}

std::string get_uptime() {
    FILE* fp = std::fopen("/proc/uptime", "r");
    if (!fp) return "unknown";

    float uptime_secs = 0;
    std::fscanf(fp, "%f", &uptime_secs);
    std::fclose(fp);

    int days = static_cast<int>(uptime_secs / 86400);
    int hours = static_cast<int>(fmod(uptime_secs / 3600, 24));
    int mins = static_cast<int>(fmod(uptime_secs / 60, 60));

    std::string result;
    if (days > 0) result += std::to_string(days) + " days, ";
    if (hours > 0 || days > 0) result += std::to_string(hours) + " hours, ";
    result += std::to_string(mins) + " mins";
    return result;
}

int get_package_count() {
    int total = 0;

    // Gentoo (emerge)
    DIR* d = opendir("/var/db/pkg");
    if (d) {
        struct dirent* e;
        while ((e = readdir(d)) != nullptr) {
            if (e->d_name[0] == '.') continue;
            char path[256];
            std::snprintf(path, sizeof(path), "/var/db/pkg/%s", e->d_name);
            total += count_subdirs(path);
        }
        closedir(d);
        if (total > 0) return total;
    }

    // Arch (pacman)
    total = count_subdirs("/var/lib/pacman/local");
    if (total > 0) return total;

    // Debian (dpkg)
    total = count_file_lines("/var/lib/dpkg/status", "Package:");
    if (total > 0) return total;

    // Void (xbps)
    total = count_subdirs("/var/db/xbps");
    if (total > 0) return total;

    // Alpine (apk)
    total = count_file_lines("/lib/apk/db/installed", "P:");
    if (total > 0) return total;

    // Fallback: fastfetch
    auto ff = run_command("fastfetch --format '{pkg.numPackages}' --pipe false 2>/dev/null");
    if (!ff.empty()) {
        try { return std::stoi(ff); }
        catch (...) {}
    }

    return 0;
}

std::string get_shell_info() {
    const char* shell = std::getenv("SHELL");
    if (!shell) return "unknown";

    std::string cmd = std::string(shell) + " --version 2>/dev/null";
    auto ver = run_command(cmd.c_str());
    if (!ver.empty()) {
        // Extract first line
        auto nl = ver.find('\n');
        if (nl != std::string::npos) ver = ver.substr(0, nl);
        return ver;
    }
    return shell;
}

std::string get_locale() {
    const char* lang = std::getenv("LANG");
    if (!lang) lang = std::getenv("LC_ALL");
    return lang ? lang : "unknown";
}

CpuInfo get_cpu_info() {
    CpuInfo info;

    FILE* fp = std::fopen("/proc/cpuinfo", "r");
    if (!fp) return info;

    char buf[512];
    while (std::fgets(buf, sizeof(buf), fp)) {
        if (std::strncmp(buf, "model name", 10) == 0 && info.model.empty()) {
            const char* p = std::strstr(buf, ": ");
            if (p) {
                p += 2;
                const char* end = p;
                while (*end && *end != '\n') end++;
                info.model.assign(p, end);
            }
        }
        if (std::strncmp(buf, "cpu cores", 9) == 0) {
            std::sscanf(buf + 11, "%d", &info.cores);
        }
        if (std::strncmp(buf, "siblings", 8) == 0) {
            std::sscanf(buf + 10, "%d", &info.threads);
        }
    }
    std::fclose(fp);

    if (info.threads == 0) info.threads = info.cores;
    if (info.cores == 0) info.cores = 1;

    // Try to get frequency
    DIR* d = opendir("/sys/devices/system/cpu/cpufreq");
    if (d) {
        struct dirent* e;
        while ((e = readdir(d)) != nullptr) {
            if (std::strncmp(e->d_name, "policy", 6) != 0) continue;
            char path[256];
            std::snprintf(path, sizeof(path),
                         "/sys/devices/system/cpu/cpufreq/%s/cpuinfo_max_freq",
                         e->d_name);
            FILE* fp2 = std::fopen(path, "r");
            if (fp2) {
                long khz = 0;
                std::fscanf(fp2, "%ld", &khz);
                std::fclose(fp2);
                float mhz = khz / 1000.0f;
                if (mhz > info.freq_mhz) info.freq_mhz = mhz;
                break;
            }
        }
        closedir(d);
    }

    // Fallback: ARM device tree
    if (info.freq_mhz == 0) {
        auto model = read_first_line("/proc/device-tree/model");
        if (!model.empty() && info.model.empty())
            info.model = model;
    }

    return info;
}

GpuInfo get_gpu_info() {
    GpuInfo info;

    auto ff = run_command("lspci -d ::0300 2>/dev/null | head -1 | sed 's/.*: //'");
    if (!ff.empty()) {
        info.model = ff;
        return info;
    }

    // Try /sys/class/drm
    DIR* d = opendir("/sys/class/drm");
    if (d) {
        struct dirent* e;
        while ((e = readdir(d)) != nullptr) {
            if (std::strncmp(e->d_name, "card", 4) != 0) continue;
            if (std::strchr(e->d_name, '-') != nullptr) continue;
            char path[256];
            std::snprintf(path, sizeof(path), "/sys/class/drm/%s/device/uevent", e->d_name);
            FILE* fp = std::fopen(path, "r");
            if (fp) {
                char buf[512];
                while (std::fgets(buf, sizeof(buf), fp)) {
                    if (std::strncmp(buf, "PCI_ID=", 7) == 0) {
                        // Parse vendor:device
                        unsigned vendor = 0, device = 0;
                        std::sscanf(buf + 7, "%X:%X", &vendor, &device);
                        // Could look up name but for now just note it
                    }
                }
                std::fclose(fp);
            }
            break;
        }
        closedir(d);
    }

    // Fallback: ARM device tree
    auto model = read_first_line("/proc/device-tree/model");
    if (!model.empty()) {
        info.model = model;
        return info;
    }

    info.model = "unknown";
    return info;
}

MemoryInfo get_memory_info() {
    MemoryInfo info;

    FILE* fp = std::fopen("/proc/meminfo", "r");
    if (!fp) return info;

    char buf[512];
    while (std::fgets(buf, sizeof(buf), fp)) {
        long val = 0;
        if (std::sscanf(buf, "MemTotal: %ld kB", &val) == 1)
            info.total_mb = val / 1024.0f;
        else if (std::sscanf(buf, "MemAvailable: %ld kB", &val) == 1)
            info.used_mb = (info.total_mb * 1024.0f - val) / 1024.0f;
        else if (std::sscanf(buf, "MemFree: %ld kB", &val) == 1) {
            if (info.used_mb == 0 && info.total_mb > 0)
                info.used_mb = (info.total_mb * 1024.0f - val) / 1024.0f;
        }
        else if (std::sscanf(buf, "SwapTotal: %ld kB", &val) == 1)
            info.swap_total_mb = val / 1024.0f;
        else if (std::sscanf(buf, "SwapFree: %ld kB", &val) == 1)
            info.swap_used_mb = (info.swap_total_mb * 1024.0f - val) / 1024.0f;
    }
    std::fclose(fp);
    return info;
}

DiskInfo get_disk_info() {
    DiskInfo info;
    info.mount = "/";

    struct statvfs st;
    if (statvfs("/", &st) == 0) {
        unsigned long block_size = st.f_frsize ? st.f_frsize : st.f_bsize;
        info.total_gb = static_cast<float>(st.f_blocks * block_size) / (1024.0f * 1024.0f * 1024.0f);
        info.used_gb = static_cast<float>((st.f_blocks - st.f_bfree) * block_size) / (1024.0f * 1024.0f * 1024.0f);
    }
    return info;
}

BatteryInfo get_battery_info() {
    BatteryInfo info;
    info.capacity = -1;

    DIR* d = opendir("/sys/class/power_supply");
    if (!d) return info;

    struct dirent* e;
    while ((e = readdir(d)) != nullptr) {
        if (e->d_name[0] == '.') continue;

        char path[256];
        int val = 0;

        // Try capacity
        std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity", e->d_name);
        FILE* fp = std::fopen(path, "r");
        if (fp) {
            std::fscanf(fp, "%d", &val);
            std::fclose(fp);
            if (val > 0 && val <= 100) {
                info.capacity = val;

                // Get energy/charge info
                for (const char* suffix : {"energy_now", "charge_now"}) {
                    std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/%s",
                                 e->d_name, suffix);
                    fp = std::fopen(path, "r");
                    if (fp) { std::fscanf(fp, "%d", &val); std::fclose(fp);
                        if (suffix[0] == 'e') info.energy_now = val;
                        else info.charge_now = val;
                    }
                }
                for (const char* suffix : {"energy_full", "charge_full"}) {
                    std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/%s",
                                 e->d_name, suffix);
                    fp = std::fopen(path, "r");
                    if (fp) { std::fscanf(fp, "%d", &val); std::fclose(fp);
                        if (suffix[0] == 'e') info.energy_full = val;
                        else info.charge_full = val;
                    }
                }

                // Status
                std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", e->d_name);
                fp = std::fopen(path, "r");
                if (fp) {
                    char buf[64];
                    if (std::fgets(buf, sizeof(buf), fp)) {
                        char* p = buf;
                        while (*p && *p != '\n') p++;
                        *p = '\0';
                        info.status = buf;
                    }
                    std::fclose(fp);
                }
                break;
            }
        }
    }
    closedir(d);
    return info;
}

DisplayInfo get_display_info() {
    DisplayInfo info;

    DIR* d = opendir("/sys/class/drm");
    if (!d) return info;

    struct dirent* e;
    while ((e = readdir(d)) != nullptr) {
        if (std::strncmp(e->d_name, "card", 4) != 0) continue;
        const char* dash = std::strchr(e->d_name, '-');
        if (!dash) continue;

        char path[256];

        // Check status
        std::snprintf(path, sizeof(path), "/sys/class/drm/%s/status", e->d_name);
        FILE* fp = std::fopen(path, "r");
        if (fp) {
            char buf[64];
            if (std::fgets(buf, sizeof(buf), fp)) {
                char* p = buf;
                while (*p && *p != '\n') p++;
                *p = '\0';
                info.status = buf;
            }
            std::fclose(fp);

            if (info.status == "connected") {
                info.name = dash + 1;

                // Get preferred mode
                std::snprintf(path, sizeof(path), "/sys/class/drm/%s/modes", e->d_name);
                fp = std::fopen(path, "r");
                if (fp) {
                    if (std::fgets(buf, sizeof(buf), fp)) {
                        char* p = buf;
                        while (*p && *p != '\n') p++;
                        *p = '\0';
                        info.resolution = buf;
                    }
                    std::fclose(fp);
                }
                break;
            }
        }
    }
    closedir(d);
    return info;
}

std::string get_wm_info() {
    // Check Wayland
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    if (wayland) {
        const char* xdg = std::getenv("XDG_CURRENT_DESKTOP");
        if (xdg) return xdg;

        // Try hyprland
        const char* hipc = std::getenv("HYPRLAND_INSTANCE_SIGNATURE");
        if (hipc) return "Hyprland";

        return "Wayland";
    }

    // Check X11
    const char* xdg = std::getenv("XDG_CURRENT_DESKTOP");
    if (xdg) return xdg;

    const char* xsession = std::getenv("XDG_SESSION_TYPE");
    if (xsession && std::string(xsession) == "x11") {
        // Try to find WM from /proc
        DIR* proc = opendir("/proc");
        if (proc) {
            struct dirent* e;
            while ((e = readdir(proc)) != nullptr) {
                if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
                char path[256];
                std::snprintf(path, sizeof(path), "/proc/%s/comm", e->d_name);
                FILE* fp = std::fopen(path, "r");
                if (fp) {
                    char buf[128];
                    if (std::fgets(buf, sizeof(buf), fp)) {
                        char* p = buf;
                        while (*p && *p != '\n') p++;
                        *p = '\0';
                        if (std::string(buf) == "Xorg" ||
                            std::string(buf) == "XWayland") {
                            // Read WM from xprop or similar
                        }
                    }
                    std::fclose(fp);
                }
            }
            closedir(proc);
        }
        return "X11";
    }

    return "unknown";
}

std::string get_theme_info() {
    const char* home = std::getenv("HOME");
    if (!home) return "unknown";

    char path[512];
    std::snprintf(path, sizeof(path), "%s/.config/gtk-3.0/settings.ini", home);
    FILE* fp = std::fopen(path, "r");
    if (!fp) return "unknown";

    char buf[512];
    while (std::fgets(buf, sizeof(buf), fp)) {
        if (std::strncmp(buf, "gtk-theme-name=", 15) == 0) {
            char* p = buf + 15;
            char* end = p;
            while (*end && *end != '\n' && *end != '\r') end++;
            *end = '\0';
            std::fclose(fp);
            return p;
        }
    }
    std::fclose(fp);
    return "unknown";
}

std::string get_icons_info() {
    // Try environment variable first
    const char* icons = std::getenv("GTK_ICON_THEME");
    if (icons) return icons;

    // Try reading from settings.ini
    const char* home = std::getenv("HOME");
    if (!home) return "unknown";

    char path[512];
    std::snprintf(path, sizeof(path), "%s/.config/gtk-3.0/settings.ini", home);
    FILE* fp = std::fopen(path, "r");
    if (!fp) return "unknown";

    char buf[512];
    while (std::fgets(buf, sizeof(buf), fp)) {
        if (std::strncmp(buf, "gtk-icon-theme-name=", 20) == 0) {
            char* p = buf + 20;
            char* end = p;
            while (*end && *end != '\n' && *end != '\r') end++;
            *end = '\0';
            std::fclose(fp);
            return p;
        }
    }
    std::fclose(fp);
    return "unknown";
}

std::string get_font_info() {
    const char* home = std::getenv("HOME");
    if (!home) return "unknown";

    char path[512];
    std::snprintf(path, sizeof(path), "%s/.config/gtk-3.0/settings.ini", home);
    FILE* fp = std::fopen(path, "r");
    if (!fp) return "unknown";

    char buf[512];
    while (std::fgets(buf, sizeof(buf), fp)) {
        if (std::strncmp(buf, "gtk-font-name=", 14) == 0) {
            char* p = buf + 14;
            char* end = p;
            while (*end && *end != '\n' && *end != '\r') end++;
            *end = '\0';
            std::fclose(fp);
            return p;
        }
    }
    std::fclose(fp);
    return "unknown";
}

std::string get_terminal_info() {
    // Check env vars for known terminals
    const char* tp = std::getenv("TERM_PROGRAM");
    if (tp) return tp;

    if (std::getenv("KITTY_WINDOW_ID")) return "kitty";
    if (std::getenv("ALACRITTY_LOG")) return "Alacritty";
    if (std::getenv("WEZTERM_PANE")) return "WezTerm";
    if (std::getenv("GHOSTTY_RESOURCES_DIR")) return "Ghostty";
    if (std::getenv("TERMINAL_EMULATOR")) return "JetBrains";

    // Try reading /proc/self/comm
    char path[256];
    std::snprintf(path, sizeof(path), "/proc/%d/comm", getpid());
    FILE* fp = std::fopen(path, "r");
    if (fp) {
        char buf[128];
        if (std::fgets(buf, sizeof(buf), fp)) {
            char* p = buf;
            while (*p && *p != '\n') p++;
            *p = '\0';
            std::fclose(fp);
            return buf;
        }
        std::fclose(fp);
    }

    return "unknown";
}

std::string get_ip_address() {
    struct ifaddrs* ifa_list = nullptr;
    if (getifaddrs(&ifa_list) != 0) return "unknown";

    std::string result;
    for (struct ifaddrs* ifa = ifa_list; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;

        // Skip loopback
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr,
                  addr, sizeof(addr));
        if (std::strncmp(addr, "127.", 4) == 0) continue;

        result = addr;
        break;
    }
    freeifaddrs(ifa_list);
    return result.empty() ? "unknown" : result;
}

} // namespace rfxh::platform
