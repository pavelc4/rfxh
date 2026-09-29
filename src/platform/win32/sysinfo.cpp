#include "platform/sysinfo.hpp"

#ifdef _WIN32

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iphlpapi.h>

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

    // Try to get product name and version from registry
    char prod[256] = {};
    char disp[64] = {};
    char build_str[32] = {};
    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                      0, KEY_READ, &key) == ERROR_SUCCESS) {
        DWORD prod_size = sizeof(prod);
        RegQueryValueExA(key, "ProductName", nullptr, nullptr,
                         (LPBYTE)prod, &prod_size);
        DWORD disp_size = sizeof(disp);
        RegQueryValueExA(key, "DisplayVersion", nullptr, nullptr,
                         (LPBYTE)disp, &disp_size);
        DWORD build_size = sizeof(build_str);
        RegQueryValueExA(key, "CurrentBuildNumber", nullptr, nullptr,
                         (LPBYTE)build_str, &build_size);
        RegCloseKey(key);

        // Some Windows 11 editions still report "Windows 10" in ProductName;
        // fix that using the build number (>= 22000 means Windows 11).
        int build = std::atoi(build_str);
        if (build >= 22000 && std::strncmp(prod, "Windows 10", 10) == 0) {
            prod[8] = '1';  // "Windows 10" -> "Windows 11" (same length)
            prod[9] = '1';
        }

        info.name = prod;
        if (disp[0]) {
            info.name += " (";
            info.name += disp;
            info.name += ")";
        }
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

    // Read model/manufacturer from registry
    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "HARDWARE\\DESCRIPTION\\System\\BIOS",
                      0, KEY_READ, &key) == ERROR_SUCCESS) {
        char vendor[256] = {};
        char product[256] = {};
        DWORD s = sizeof(vendor);
        RegQueryValueExA(key, "SystemManufacturer", nullptr, nullptr,
                         (LPBYTE)vendor, &s);
        s = sizeof(product);
        RegQueryValueExA(key, "SystemProductName", nullptr, nullptr,
                         (LPBYTE)product, &s);
        RegCloseKey(key);

        std::string model;
        if (vendor[0]) { model += vendor; model += " "; }
        if (product[0]) model += product;
        if (!model.empty()) info.product = model;
    }
    if (info.product.empty()) info.product = "unknown";
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
    wchar_t buf[LOCALE_NAME_MAX_LENGTH] = {0};
    if (GetUserDefaultLocaleName(buf, LOCALE_NAME_MAX_LENGTH) > 0) {
        char narrow[LOCALE_NAME_MAX_LENGTH * 4] = {0};
        int len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, narrow,
                                      sizeof(narrow), nullptr, nullptr);
        if (len > 0)
            return narrow;
    }
    return "unknown";
}

CpuInfo get_cpu_info() {
    CpuInfo info;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    info.cores = si.dwNumberOfProcessors;
    info.threads = info.cores;

    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &key) == ERROR_SUCCESS) {
        char name[256] = {};
        DWORD name_size = sizeof(name);
        if (RegQueryValueExA(key, "ProcessorNameString", nullptr, nullptr,
                             (LPBYTE)name, &name_size) == ERROR_SUCCESS) {
            info.model = name;
            while (!info.model.empty() &&
                   (info.model.back() == ' ' || info.model.back() == '\t'))
                info.model.pop_back();
        }
        DWORD mhz = 0;
        DWORD mhz_size = sizeof(mhz);
        if (RegQueryValueExA(key, "~MHz", nullptr, nullptr,
                             (LPBYTE)&mhz, &mhz_size) == ERROR_SUCCESS)
            info.freq_mhz = static_cast<float>(mhz);
        RegCloseKey(key);
    }
    if (info.model.empty()) info.model = "unknown";
    return info;
}

GpuInfo get_gpu_info() {
    GpuInfo info;
    const char* adapter_class =
        "SYSTEM\\CurrentControlSet\\Control\\Class\\"
        "{4d36e968-e325-11ce-bfc1-08002be10318}";
    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, adapter_class, 0, KEY_READ, &key)
        == ERROR_SUCCESS) {
        DWORD idx = 0;
        while (true) {
            char subkey[64];
            DWORD subkey_size = sizeof(subkey);
            if (RegEnumKeyExA(key, idx++, subkey, &subkey_size,
                              nullptr, nullptr, nullptr, nullptr)
                != ERROR_SUCCESS)
                break;

            // Adapter keys are 4-digit numbers (0000, 0001, ...)
            bool numeric = subkey[0] && subkey[1] && subkey[2] && subkey[3]
                           && !subkey[4];
            for (int i = 0; numeric && i < 4; i++)
                if (subkey[i] < '0' || subkey[i] > '9') numeric = false;
            if (!numeric) continue;

            HKEY dev;
            if (RegOpenKeyExA(key, subkey, 0, KEY_READ, &dev)
                == ERROR_SUCCESS) {
                char desc[256] = {};
                DWORD desc_size = sizeof(desc);
                if (RegQueryValueExA(dev, "DriverDesc", nullptr, nullptr,
                                     (LPBYTE)desc, &desc_size) == ERROR_SUCCESS
                    && desc[0]) {
                    if (std::strstr(desc, "Microsoft Basic") == nullptr &&
                        std::strstr(desc, "Microsoft Virtual") == nullptr) {
                        info.model = desc;
                        RegCloseKey(dev);
                        break;
                    }
                }
                RegCloseKey(dev);
            }
        }
        RegCloseKey(key);
    }
    if (info.model.empty()) info.model = "unknown";
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
    ULONG size = 0;
    if (GetAdaptersInfo(nullptr, &size) != ERROR_BUFFER_OVERFLOW)
        return "unknown";
    PIP_ADAPTER_INFO adapters = (PIP_ADAPTER_INFO)std::malloc(size);
    if (!adapters) return "unknown";

    std::string result = "unknown";
    std::string fallback;
    if (GetAdaptersInfo(adapters, &size) == NO_ERROR) {
        for (PIP_ADAPTER_INFO a = adapters; a; a = a->Next) {
            if (a->Type == MIB_IF_TYPE_LOOPBACK) continue;
            const char* ip = a->IpAddressList.IpAddress.String;
            if (!ip[0] || std::strcmp(ip, "0.0.0.0") == 0) continue;
            if (fallback.empty()) fallback = ip;
            const char* gw = a->GatewayList.IpAddress.String;
            if (gw[0] && std::strcmp(gw, "0.0.0.0") != 0) {
                result = ip;
                break;
            }
        }
    }
    std::free(adapters);
    if (result == "unknown") result = fallback;
    return result;
}

} // namespace rfxh::platform

#endif // _WIN32
