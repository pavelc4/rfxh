#include "gather/gather.hpp"
#include <cstdio>
#include <cstring>
#include <sys/statvfs.h>

namespace rfxh::gather {

void gather_disk(GatherContext& ctx) {
    struct statvfs st;
    if (statvfs("/", &st) != 0)
        return;

    float total_gib = static_cast<float>(st.f_blocks) *
                      static_cast<float>(st.f_frsize) / (1024 * 1024 * 1024);
    float free_gib = static_cast<float>(st.f_bfree) *
                     static_cast<float>(st.f_frsize) / (1024 * 1024 * 1024);
    float used_gib = total_gib - free_gib;
    int pct = static_cast<int>(used_gib * 100 / total_gib);
    const char* color = pct >= 80 ? "31" : pct >= 50 ? "93" : "32";

    char fstype[32] = "";
    FILE* fp = std::fopen("/proc/mounts", "r");
    if (fp) {
        char buf[512];
        while (std::fgets(buf, sizeof(buf), fp)) {
            char dev[128], mnt[128], fs[32];
            if (std::sscanf(buf, "%127s %127s %31s", dev, mnt, fs) == 3) {
                if (std::strcmp(mnt, "/") == 0) {
                    std::strncpy(fstype, fs, sizeof(fstype) - 1);
                    break;
                }
            }
        }
        std::fclose(fp);
    }

    if (fstype[0])
        add_info(ctx, "Disk (/)", "%.2f GiB / %.2f GiB (\033[%sm%d%%\033[0m) - %s",
                 used_gib, total_gib, color, pct, fstype);
    else
        add_info(ctx, "Disk (/)", "%.2f GiB / %.2f GiB (\033[%sm%d%%\033[0m)", used_gib,
                 total_gib, color, pct);
}

} // namespace rfxh::gather
