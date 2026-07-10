#include "platform/sysinfo.hpp"

#ifdef _WIN32

#include <windows.h>
#include <cstdio>
#include <string>

namespace rfxh::platform {

OsInfo get_os_info() {
    OsInfo info;

    OSVERSIONINFOEXA osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    // Use RtlGetVersion (available on Windows 10+)
    using RtlGetVersion = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    auto mod = GetModuleHandleA("ntdll.dll");
    if (mod) {
        auto fn = (RtlGetVersion)GetProcAddress(mod, "RtlGetVersion");
        if (fn) {
            RTL_OSVERSIONINFOW wosvi = {};
            wosvi.dwOSVersionInfoSize = sizeof(wosvi);
            if (fn(&wosvi) == 0) {
                info.version = std::to_string(wosvi.dwMajorVersion) + "."
                             + std::to_string(wosvi.dwMinorVersion) + "."
                             + std::to_string(wosvi.dwBuildNumber);
            }
        }
    }

    // Try to get product name from registry
    char prod[256] = {};
    DWORD prod_size = sizeof(prod);
    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                      0, KEY_READ, &key) == ERROR_SUCCESS) {
        // Try "ProductName"
        if (RegQueryValueExA(key, "ProductName", nullptr, nullptr,
                            (LPBYTE)prod, &prod_size) == ERROR_SUCCESS) {
            info.name = prod;
        }
        RegCloseKey(key);
    }

    if (info.name.empty()) {
        info.name = "Windows";
        if (info.version.empty()) info.version = "unknown";
    }

    info.id = "windows";
    return info;
}

HostInfo get_host_info() {
    HostInfo info;

    char hostname[256];
    DWORD size = sizeof(hostname);
    GetComputerNameA(hostname, &size);
    info.hostname = hostname;

    // Try WMI for product name (stub for now)
    info.product = "unknown";
    return info;
}

std::string get_kernel_version() {
    OSVERSIONINFOEXA osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    using RtlGetVersion = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    auto mod = GetModuleHandleA("ntdll.dll");
    if (mod) {
        auto fn = (RtlGetVersion)GetProcAddress(mod, "RtlGetVersion");
        if (fn) {
            RTL_OSVERSIONINFOW wosvi = {};
            wosvi.dwOSVersionInfoSize = sizeof(wosvi);
            if (fn(&wosvi) == 0) {
                return "Windows " + std::to_string(wosvi.dwMajorVersion) + "."
                     + std::to_string(wosvi.dwMinorVersion) + "."
                     + std::to_string(wosvi.dwBuildNumber);
            }
        }
    }
    return "Windows";
}

std::string get_uptime() {
    ULONGLONG millis = GetTickCount64();
    int days = static_cast<int>(millis / 86400000);
    int hours = static_cast<int>((millis % 86400000) / 3600000);
    int mins = static_cast<int>((millis % 3600000) / 60000);

    std::string result;
    if (days > 0) result += std::to_string(days) + " days, ";
    if (hours > 0 || days > 0) result += std::to_string(hours) + " hours, ";
    result += std::to_string(mins) + " mins";
    return result;
}

int get_package_count() {
    // Could count via winget/choco/scoop but stub for now
    return 0;
}

std::string get_shell_info() {
    const char* spec = std::getenv("COMSPEC");
    return spec ? spec : "cmd.exe";
}

std::string get_locale() {
    char buf[LOCALE_NAME_MAX_LENGTH];
    if (GetUserDefaultLocaleName(buf, LOCALE_NAME_MAX_LENGTH) > 0)
        return buf;
    return "unknown";
}

CpuInfo get_cpu_info() {
    CpuInfo info;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    info.cores = si.dwNumberOfProcessors;
    info.threads = info.cores;
    info.model = "unknown"; // Could use WMI for model name
    return info;
}

GpuInfo get_gpu_info() {
    GpuInfo info;
    info.model = "unknown"; // Could use WMI or DXGI
    return info;
}

MemoryInfo get_memory_info() {
    MemoryInfo info;
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    if (GlobalMemoryStatusEx(&ms)) {
        info.total_mb = ms.ullTotalPhys / (1024.0f * 1024.0f);
        info.used_mb = info.total_mb - (ms.ullAvailPhys / (1024.0f * 1024.0f));
        info.swap_total_mb = ms.ullTotalPageFile / (1024.0f * 1024.0f);
        info.swap_used_mb = (ms.ullTotalPageFile - ms.ullAvailPageFile) / (1024.0f * 1024.0f);
    }
    return info;
}

DiskInfo get_disk_info() {
    DiskInfo info;
    info.mount = "C:\\";

    ULARGE_INTEGER free_bytes, total_bytes;
    if (GetDiskFreeSpaceExA("C:\\", nullptr, &total_bytes, &free_bytes)) {
        info.total_gb = total_bytes.QuadPart / (1024.0f * 1024.0f * 1024.0f);
        info.used_gb = (total_bytes.QuadPart - free_bytes.QuadPart) / (1024.0f * 1024.0f * 1024.0f);
    }
    return info;
}

BatteryInfo get_battery_info() {
    BatteryInfo info;
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps)) {
        if (sps.BatteryLifePercent != 127) {
            info.capacity = sps.BatteryLifePercent;
        }
        if (sps.BatteryFlag & 128) {
            info.capacity = -1; // No battery
        }
        info.status = (sps.ACLineStatus == 1) ? "Charging" : "Discharging";
    }
    return info;
}

DisplayInfo get_display_info() {
    DisplayInfo info;
    info.name = "primary";
    info.status = "connected";

    DEVMODEA dm = {};
    dm.dmSize = sizeof(dm);
    if (EnumDisplaySettingsA(nullptr, ENUM_CURRENT_SETTINGS, &dm)) {
        info.resolution = std::to_string(dm.dmPelsWidth) + "x" + std::to_string(dm.dmPelsHeight);
    }
    return info;
}

std::string get_wm_info() {
    // Windows Desktop Window Manager is always the WM
    return "DWM";
}

std::string get_theme_info() {
    // Check registry for theme
    HKEY key;
    DWORD theme = 0;
    DWORD size = sizeof(theme);
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
                      "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &key) == ERROR_SUCCESS) {
        if (RegQueryValueExA(key, "AppsUseLightTheme", nullptr, nullptr,
                            (LPBYTE)&theme, &size) == ERROR_SUCCESS) {
            RegCloseKey(key);
            return theme == 0 ? "Dark" : "Light";
        }
        RegCloseKey(key);
    }
    return "unknown";
}

std::string get_icons_info() {
    return "Windows";
}

std::string get_font_info() {
    return "Segoe UI";
}

std::string get_terminal_info() {
    // Check if running in Windows Terminal
    const char* wt = std::getenv("WT_SESSION");
    if (wt) return "Windows Terminal";

    // Check ConEmu
    const char* conemu = std::getenv("ConEmuPID");
    if (conemu) return "ConEmu";

    // Check for other terminals
    if (std::getenv("TERM_PROGRAM")) return std::getenv("TERM_PROGRAM");

    return "cmd.exe";
}

std::string get_ip_address() {
    // Stub - could use GetAdaptersInfo
    return "unknown";
}

} // namespace rfxh::platform

#endif // _WIN32
