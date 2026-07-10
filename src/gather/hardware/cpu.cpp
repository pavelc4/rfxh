#include "gather/gather.hpp"
#include <cstdio>
#include <cstring>
#include <dirent.h>

namespace rfxh::gather {

void gather_cpu(GatherContext& ctx) {
    char name[128] = "";
    int cores = 0;
    float max_ghz = 0;

    FILE* fp = std::fopen("/proc/cpuinfo", "r");
    if (fp) {
        char buf[256];
        while (std::fgets(buf, sizeof(buf), fp)) {
            if (!name[0] && std::strncmp(buf, "model name", 10) == 0) {
                char* val = std::strchr(buf, ':');
                if (val) {
                    val++;
                    while (*val == ' ')
                        val++;
                    int len = std::strlen(val);
                    while (len > 0 && (val[len - 1] == '\n' || val[len - 1] == '\r'))
                        len--;
                    if (len > 0 && len < static_cast<int>(sizeof(name))) {
                        std::memcpy(name, val, len);
                        name[len] = '\0';
                    }
                }
            }
            if (std::strncmp(buf, "processor", 9) == 0)
                cores++;
        }
        std::fclose(fp);
    }

    if (!name[0]) {
        char model[128] = "";
        fp = std::fopen("/proc/device-tree/model", "r");
        if (fp) {
            if (std::fgets(model, sizeof(model), fp)) {
                int len = std::strlen(model);
                while (len > 0 && (model[len - 1] == '\n' || model[len - 1] == '\0'))
                    len--;
                model[len] = '\0';
            }
            std::fclose(fp);
        }
        char* paren = std::strchr(model, '(');
        if (paren) {
            paren++;
            char* comma = std::strchr(paren, ',');
            char* end = comma ? comma : std::strchr(paren, ')');
            if (end) {
                std::snprintf(name, sizeof(name), "Apple %.*s",
                              static_cast<int>(end - paren), paren);
            }
        }
    }

    {
        DIR* cpufreq = opendir("/sys/devices/system/cpu/cpufreq");
        if (cpufreq) {
            struct dirent* ent;
            long max_khz = 0;
            while ((ent = readdir(cpufreq))) {
                if (std::strncmp(ent->d_name, "policy", 6) != 0)
                    continue;
                char path[128];
                std::snprintf(path, sizeof(path),
                              "/sys/devices/system/cpu/cpufreq/%s/cpuinfo_max_freq",
                              ent->d_name);
                FILE* f = std::fopen(path, "r");
                if (!f)
                    continue;
                long khz = 0;
                if (std::fscanf(f, "%ld", &khz) == 1 && khz > max_khz)
                    max_khz = khz;
                std::fclose(f);
            }
            closedir(cpufreq);
            if (max_khz > 0)
                max_ghz = max_khz / 1000000.0f;
        }
    }

    if (name[0]) {
        if (cores > 0 && max_ghz > 0)
            add_info(ctx, "CPU", "%s (%d) @ %.2f GHz", name, cores, max_ghz);
        else if (cores > 0)
            add_info(ctx, "CPU", "%s (%d)", name, cores);
        else
            add_info(ctx, "CPU", "%s", name);
    }
}

} // namespace rfxh::gather
