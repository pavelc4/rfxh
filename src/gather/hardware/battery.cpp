#include "gather/gather.hpp"
#include <cstdio>
#include <cstring>
#include <dirent.h>

namespace rfxh::gather {

void gather_battery(GatherContext& ctx) {
    char bat_name[64] = "";
    DIR* psd = opendir("/sys/class/power_supply");
    if (!psd)
        return;
    struct dirent* psent;
    while ((psent = readdir(psd))) {
        if (psent->d_name[0] == '.')
            continue;
        char path[256];
        std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity",
                      psent->d_name);
        FILE* test = std::fopen(path, "r");
        if (test) {
            std::fclose(test);
            std::strncpy(bat_name, psent->d_name, sizeof(bat_name) - 1);
            break;
        }
    }
    closedir(psd);
    if (!bat_name[0])
        return;

    char path[256];
    FILE* fp;
    int capacity = -1;
    char status[32] = "";

    long energy_now = 0, energy_full = 0;
    std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/energy_now",
                  bat_name);
    fp = std::fopen(path, "r");
    if (fp) {
        if (std::fscanf(fp, "%ld", &energy_now) != 1)
            energy_now = 0;
        std::fclose(fp);
    }
    std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/energy_full",
                  bat_name);
    fp = std::fopen(path, "r");
    if (fp) {
        if (std::fscanf(fp, "%ld", &energy_full) != 1)
            energy_full = 0;
        std::fclose(fp);
    }
    if (energy_full > 0)
        capacity = static_cast<int>(energy_now * 100 / energy_full);

    if (capacity < 0) {
        long charge_now = 0, charge_full = 0;
        std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/charge_now",
                      bat_name);
        fp = std::fopen(path, "r");
        if (fp) {
            if (std::fscanf(fp, "%ld", &charge_now) != 1)
                charge_now = 0;
            std::fclose(fp);
        }
        std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/charge_full",
                      bat_name);
        fp = std::fopen(path, "r");
        if (fp) {
            if (std::fscanf(fp, "%ld", &charge_full) != 1)
                charge_full = 0;
            std::fclose(fp);
        }
        if (charge_full > 0)
            capacity = static_cast<int>(charge_now * 100 / charge_full);
    }

    if (capacity < 0) {
        std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity",
                      bat_name);
        fp = std::fopen(path, "r");
        if (fp) {
            if (std::fscanf(fp, "%d", &capacity) != 1)
                capacity = -1;
            std::fclose(fp);
        }
    }

    std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", bat_name);
    fp = std::fopen(path, "r");
    if (fp) {
        if (std::fgets(status, sizeof(status), fp)) {
            int len = std::strlen(status);
            while (len > 0 && (status[len - 1] == '\n' || status[len - 1] == '\r'))
                status[--len] = '\0';
        }
        std::fclose(fp);
    }

    char time_str[64] = "";
    if (energy_now > 0) {
        long power_now = 0;
        std::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/power_now",
                      bat_name);
        fp = std::fopen(path, "r");
        if (fp) {
            if (std::fscanf(fp, "%ld", &power_now) != 1)
                power_now = 0;
            std::fclose(fp);
        }
        if (power_now < 0)
            power_now = -power_now;
        if (power_now > 0) {
            int mins_left = static_cast<int>(static_cast<float>(energy_now) /
                                             power_now * 60);
            int hours = mins_left / 60;
            int mins = mins_left % 60;
            if (hours > 0)
                std::snprintf(time_str, sizeof(time_str),
                              "%d hour%s, %d min%s remaining",
                              hours, hours == 1 ? "" : "s", mins, mins == 1 ? "" : "s");
            else
                std::snprintf(time_str, sizeof(time_str), "%d min%s remaining", mins,
                              mins == 1 ? "" : "s");
        }
    }

    if (capacity >= 0) {
        const char* color = capacity >= 50 ? "32" : capacity >= 20 ? "93" : "31";
        if (time_str[0] && status[0])
            add_info(ctx, "Battery", "\033[%sm%d%%\033[0m (%s) [%s]", color, capacity,
                     time_str, status);
        else if (status[0])
            add_info(ctx, "Battery", "\033[%sm%d%%\033[0m [%s]", color, capacity, status);
        else
            add_info(ctx, "Battery", "\033[%sm%d%%\033[0m", color, capacity);
    }
}

} // namespace rfxh::gather
