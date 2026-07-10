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

#ifdef __APPLE__
#include <sys/sysctl.h>
#include <mach/mach_host.h>
#include <mach/host_priv.h>
#endif

namespace rfxh::platform {

// Helper: read first line from file
static std::string read_first_line(const char* path) {
    FILE* fp = std::fopen(path, "r");
    if (!fp) return "";
    char buf[512];
    if (std::fgets(buf, sizeof(buf), fp)) {
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

#ifdef __APPLE__
// macOS: read sysctl string
static std::string sysctl_str(const char* name) {
    size_t size = 0;
    if (sysctlbyname(name, nullptr, &size, nullptr, 0) != 0 || size == 0)
        return "";
    std::string buf(size, '\0');
    if (sysctlbyname(name, buf.data(), &size, nullptr, 0) != 0)
        return "";
    // Trim trailing null/newline
    while (!buf.empty() && (buf.back() == '\0' || buf.back() == '\n' || buf.back() == '\r'))
        buf.pop_back();
    return buf;
}

// macOS: read sysctl int64
static long sysctl_int(const char* name) {
    long val = 0;
    size_t size = sizeof(val);
    sysctlbyname(name, &val, &size, nullptr, 0);
    return val;
}
#endif

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

#ifdef __APPLE__
    info.name = sysctl_str("kern.osproductname");
    if (info.name.empty()) info.name = "macOS";
    info.version = sysctl_str("kern.osproductversion");
    info.id = "darwin";
    info.id_like = "bsd";
#else
    info.name = read_os_release_key("NAME");
    info.version = read_os_release_key("VERSION_ID");
    info.id = read_os_release_key("ID");
    info.id_like = read_os_release_key("ID_LIKE");
    if (info.name.empty()) {
        info.name = "Linux";
        info.id = "linux";
    }
#endif
    return info;
}

HostInfo get_host_info() {
    HostInfo info;

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
        info.hostname = hostname;

#ifdef __APPLE__
    info.product = sysctl_str("hw.model");
#else
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
    auto ff = run_command("fastfetch --format '{system.productName}' --pipe false 2>/dev/null");
    if (!ff.empty()) info.product = ff;
#endif
    return info;
}

std::string get_kernel_version() {
    struct utsname u;
    if (uname(&u) == 0)
        return std::string(u.release) + " " + u.machine;
    return "unknown";
}

std::string get_uptime() {
#ifdef __APPLE__
    // macOS: get boot time from sysctl
    long boot_time = sysctl_int("kern.boottime");
    if (boot_time == 0) return "unknown";

    long now = 0;
    time(&now);
    float uptime_secs = static_cast<float>(now - boot_time);
#else
    FILE* fp = std::fopen("/proc/uptime", "r");
    if (!fp) return "unknown";
    float uptime_secs = 0;
    std::fscanf(fp, "%f", &uptime_secs);
    std::fclose(fp);
#endif

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
#ifdef __APPLE__
    // macOS: count Homebrew packages
    auto ff = run_command("brew list 2>/dev/null | wc -l");
    if (!ff.empty()) {
        try { return std::stoi(ff); }
        catch (...) {}
    }
    // Count MacPorts
    int macports = count_subdirs("/opt/local/var/macports/software");
    if (macports > 0) return macports;
    return 0;
#else
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

    auto ff = run_command("fastfetch --format '{pkg.numPackages}' --pipe false 2>/dev/null");
    if (!ff.empty()) {
        try { return std::stoi(ff); }
        catch (...) {}
    }
    return 0;
#endif
}

std::string get_shell_info() {
    const char* shell = std::getenv("SHELL");
    if (!shell) return "unknown";

    std::string cmd = std::string(shell) + " --version 2>/dev/null";
    auto ver = run_command(cmd.c_str());
    if (!ver.empty()) {
        auto nl = ver.find('\n');
        if (nl != std::string::npos) ver = ver.substr(0, nl);
        return ver;
    }
    return shell;
}

std::string get_locale() {
    const char* lang = std::getenv("LANG");
    if (!lang) lang = std::getenv("LC_ALL");
#ifdef __APPLE__
    if (!lang) {
        // macOS: read from defaults
        auto loc = run_command("defaults read -g AppleLocale 2>/dev/null");
        if (!loc.empty()) return loc;
    }
#endif
    return lang ? lang : "unknown";
}

CpuInfo get_cpu_info() {
    CpuInfo info;

#ifdef __APPLE__
    info.model = sysctl_str("machdep.cpu.brand_string");
    info.cores = static_cast<int>(sysctl_int("hw.physicalcpu"));
    info.threads = static_cast<int>(sysctl_int("hw.logicalcpu"));
    long freq_hz = sysctl_int("hw.cpufrequency");
    if (freq_hz > 0) info.freq_mhz = freq_hz / 1000000.0f;
#else
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
        if (std::strncmp(buf, "cpu cores", 9) == 0)
            std::sscanf(buf + 11, "%d", &info.cores);
        if (std::strncmp(buf, "siblings", 8) == 0)
            std::sscanf(buf + 10, "%d", &info.threads);
    }
    std::fclose(fp);

    if (info.threads == 0) info.threads = info.cores;
    if (info.cores == 0) info.cores = 1;

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

    if (info.freq_mhz == 0) {
        auto model = read_first_line("/proc/device-tree/model");
        if (!model.empty() && info.model.empty())
            info.model = model;
    }
#endif
    return info;
}

GpuInfo get_gpu_info() {
    GpuInfo info;

#ifdef __APPLE__
    // macOS: use system_profiler
    auto ff = run_command("system_profiler SPDisplaysDataType 2>/dev/null | grep 'Chipset Model' | head -1 | sed 's/.*: //'");
    if (!ff.empty()) {
        info.model = ff;
        return info;
    }
    info.model = sysctl_str("hw.model");
#else
    auto ff = run_command("lspci -d ::0300 2>/dev/null | head -1 | sed 's/.*: //'");
    if (!ff.empty()) {
        info.model = ff;
        return info;
    }

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
                        unsigned vendor = 0, device = 0;
                        std::sscanf(buf + 7, "%X:%X", &vendor, &device);
                    }
                }
                std::fclose(fp);
            }
            break;
        }
        closedir(d);
    }

    auto model = read_first_line("/proc/device-tree/model");
    if (!model.empty()) {
        info.model = model;
        return info;
    }
    info.model = "unknown";
#endif
    return info;
}

MemoryInfo get_memory_info() {
    MemoryInfo info;

#ifdef __APPLE__
    // macOS: use sysctl for total, vm_stat for used
    long total_bytes = sysctl_int("hw.memsize");
    info.total_mb = total_bytes / (1024.0f * 1024.0f);

    // vm_stat for free pages
    vm_statistics_data_t vm_stat;
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_VM_INFO,
                       reinterpret_cast<integer_t*>(&vm_stat), &count) == KERN_SUCCESS) {
        long free_bytes = vm_stat.free_count * vm_page_size;
        info.used_mb = (total_bytes - free_bytes) / (1024.0f * 1024.0f);
    }
    // Swap: use sysctl
    long swap_total = sysctl_int("vm.swapusage");
    long swap_used = sysctl_int("vm.swapusage"); // swapusage returns total+used
    // Actually vm.swapusage format is "total = X, used = Y, ..."
    auto swap_line = run_command("sysctl vm.swapusage 2>/dev/null");
    if (!swap_line.empty()) {
        float st = 0, su = 0;
        if (std::sscanf(swap_line.c_str(), "vm.swapusage: total = %f M, used = %f M", &st, &su) == 2) {
            info.swap_total_mb = st;
            info.swap_used_mb = su;
        }
    }
#else
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
#endif
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

#ifdef __APPLE__
    // macOS: use ioreg for battery info
    auto ff = run_command("ioreg -r -c AppleSmartBattery 2>/dev/null | grep 'MaxCapacity' | head -1 | sed 's/.*= //'");
    if (!ff.empty()) {
        int max_cap = 0;
        try { max_cap = std::stoi(ff); } catch (...) {}
        auto ff2 = run_command("ioreg -r -c AppleSmartBattery 2>/dev/null | grep 'CurrentCapacity' | head -1 | sed 's/.*= //'");
        int cur_cap = 0;
        if (!ff2.empty()) {
            try { cur_cap = std::stoi(ff2); } catch (...) {}
        }
        if (max_cap > 0) {
            info.capacity = (cur_cap * 100) / max_cap;
            info.status = (info.capacity >= 100) ? "Full" : "Discharging";
        }
    }
#else
    DIR* d = opendir("/sys/class/power_supply");
    if (!d) return info;

    struct dirent* e;
    while ((e = readdir(d)) != nullptr) {
        if (e->d_name[0] == '.') continue;

        char path[256];
        int val = 0;

        std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity", e->d_name);
        FILE* fp = std::fopen(path, "r");
        if (fp) {
            std::fscanf(fp, "%d", &val);
            std::fclose(fp);
            if (val > 0 && val <= 100) {
                info.capacity = val;

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
#endif
    return info;
}

DisplayInfo get_display_info() {
    DisplayInfo info;

#ifdef __APPLE__
    // macOS: use system_profiler
    auto name = run_command("system_profiler SPDisplaysDataType 2>/dev/null | grep 'Display Type' | head -1 | sed 's/.*: //'");
    if (!name.empty()) info.name = name;
    else info.name = "Built-in";

    auto res = run_command("system_profiler SPDisplaysDataType 2>/dev/null | grep 'Resolution' | head -1 | sed 's/.*: //' | sed 's/ (.*//'");
    if (!res.empty()) info.resolution = res;
    info.status = "connected";
#else
    DIR* d = opendir("/sys/class/drm");
    if (!d) return info;

    struct dirent* e;
    while ((e = readdir(d)) != nullptr) {
        if (std::strncmp(e->d_name, "card", 4) != 0) continue;
        const char* dash = std::strchr(e->d_name, '-');
        if (!dash) continue;

        char path[256];
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
#endif
    return info;
}

std::string get_wm_info() {
#ifdef __APPLE__
    return "Quartz Compositor";
#else
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    if (wayland) {
        const char* xdg = std::getenv("XDG_CURRENT_DESKTOP");
        if (xdg) return xdg;
        const char* hipc = std::getenv("HYPRLAND_INSTANCE_SIGNATURE");
        if (hipc) return "Hyprland";
        return "Wayland";
    }

    const char* xdg = std::getenv("XDG_CURRENT_DESKTOP");
    if (xdg) return xdg;

    const char* xsession = std::getenv("XDG_SESSION_TYPE");
    if (xsession && std::string(xsession) == "x11") {
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
                    }
                    std::fclose(fp);
                }
            }
            closedir(proc);
        }
        return "X11";
    }
    return "unknown";
#endif
}

std::string get_theme_info() {
#ifdef __APPLE__
    auto style = run_command("defaults read -g AppleInterfaceStyle 2>/dev/null");
    if (!style.empty()) return style;
    return "Light";
#else
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
#endif
}

std::string get_icons_info() {
#ifdef __APPLE__
    return "macOS";
#else
    const char* icons = std::getenv("GTK_ICON_THEME");
    if (icons) return icons;

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
#endif
}

std::string get_font_info() {
#ifdef __APPLE__
    auto font = run_command("defaults read -g NSFixedPitchFont 2>/dev/null");
    if (!font.empty()) return font;
    return "SF Mono";
#else
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
#endif
}

std::string get_terminal_info() {
    const char* tp = std::getenv("TERM_PROGRAM");
    if (tp) return tp;

    if (std::getenv("KITTY_WINDOW_ID")) return "kitty";
    if (std::getenv("ALACRITTY_LOG")) return "Alacritty";
    if (std::getenv("WEZTERM_PANE")) return "WezTerm";
    if (std::getenv("GHOSTTY_RESOURCES_DIR")) return "Ghostty";
    if (std::getenv("TERMINAL_EMULATOR")) return "JetBrains";

#ifdef __APPLE__
    // macOS: no /proc, try to get process name via other means
    return "unknown";
#else
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
#endif
}

std::string get_ip_address() {
    struct ifaddrs* ifa_list = nullptr;
    if (getifaddrs(&ifa_list) != 0) return "unknown";

    std::string result;
    for (struct ifaddrs* ifa = ifa_list; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;

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
