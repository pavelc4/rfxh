#pragma once

#include <string>

namespace rfxh::platform {

struct OsInfo {
    std::string name;       // e.g. "Ubuntu", "Windows 11", "macOS"
    std::string version;    // e.g. "22.04", "22H2", "14.2"
    std::string id;         // e.g. "ubuntu", "windows", "darwin"
    std::string id_like;    // e.g. "debian" (for distro fallback)
};

struct HostInfo {
    std::string hostname;
    std::string product;    // motherboard/laptop model
};

struct CpuInfo {
    std::string model;
    int cores = 0;
    int threads = 0;
    float freq_mhz = 0;    // max freq in MHz
};

struct MemoryInfo {
    float total_mb = 0;
    float used_mb = 0;
    float swap_total_mb = 0;
    float swap_used_mb = 0;
};

struct DiskInfo {
    float total_gb = 0;
    float used_gb = 0;
    std::string mount;
};

struct BatteryInfo {
    int capacity = -1;      // percentage, -1 = no battery
    int energy_now = 0;     // microwatt-hours
    int energy_full = 0;
    int charge_now = 0;     // microamp-hours
    int charge_full = 0;
    int power_now = 0;
    std::string status;     // "Charging", "Discharging", "Full"
};

struct GpuInfo {
    std::string model;
    std::string driver;
};

struct DisplayInfo {
    std::string name;
    std::string resolution;
    std::string status;
};

// System info functions
OsInfo get_os_info();
HostInfo get_host_info();
std::string get_kernel_version();
std::string get_uptime();
int get_package_count();
std::string get_shell_info();
std::string get_locale();
CpuInfo get_cpu_info();
GpuInfo get_gpu_info();
MemoryInfo get_memory_info();
DiskInfo get_disk_info();
BatteryInfo get_battery_info();
DisplayInfo get_display_info();
std::string get_wm_info();
std::string get_theme_info();
std::string get_icons_info();
std::string get_font_info();
std::string get_terminal_info();
std::string get_ip_address();

} // namespace rfxh::platform
