#include "gather/gather.hpp"
#include <cstdio>
#include <cstring>

namespace rfxh::gather {

void gather_memory(GatherContext& ctx) {
    long long total = 0, avail = 0;
    FILE* fp = std::fopen("/proc/meminfo", "r");
    if (!fp)
        return;
    char buf[128];
    while (std::fgets(buf, sizeof(buf), fp)) {
        if (std::strncmp(buf, "MemTotal:", 9) == 0)
            std::sscanf(buf + 9, " %lld", &total);
        else if (std::strncmp(buf, "MemAvailable:", 13) == 0)
            std::sscanf(buf + 13, " %lld", &avail);
    }
    std::fclose(fp);
    if (total <= 0)
        return;

    float used_gib = (total - avail) / 1048576.0f;
    float total_gib = total / 1048576.0f;
    int pct = static_cast<int>((total - avail) * 100 / total);

    const char* color = pct >= 80 ? "31" : pct >= 50 ? "93" : "32";

    add_info(ctx, "Memory", "%.2f GiB / %.2f GiB (\033[%sm%d%%\033[0m)", used_gib,
             total_gib, color, pct);
}

void gather_swap(GatherContext& ctx) {
    long long total = 0, free_s = 0;
    FILE* fp = std::fopen("/proc/meminfo", "r");
    if (!fp)
        return;
    char buf[128];
    while (std::fgets(buf, sizeof(buf), fp)) {
        if (std::strncmp(buf, "SwapTotal:", 10) == 0)
            std::sscanf(buf + 10, " %lld", &total);
        else if (std::strncmp(buf, "SwapFree:", 9) == 0)
            std::sscanf(buf + 9, " %lld", &free_s);
    }
    std::fclose(fp);
    if (total <= 0)
        return;

    long long used = total - free_s;
    int pct = static_cast<int>(used * 100 / total);
    const char* color = pct >= 80 ? "31" : pct >= 50 ? "93" : "32";

    if (total >= 1048576)
        add_info(ctx, "Swap", "%.2f GiB / %.2f GiB (\033[%sm%d%%\033[0m)",
                 used / 1048576.0f, total / 1048576.0f, color, pct);
    else
        add_info(ctx, "Swap", "%.2f MiB / %.2f MiB (\033[%sm%d%%\033[0m)",
                 used / 1024.0f, total / 1024.0f, color, pct);
}

} // namespace rfxh::gather
