#include "gather/gather.hpp"
#include <cstdio>

namespace rfxh::gather {

void gather_uptime(GatherContext& ctx) {
    FILE* fp = std::fopen("/proc/uptime", "r");
    if (!fp)
        return;

    double secs = 0;
    if (std::fscanf(fp, "%lf", &secs) != 1)
        secs = 0;
    std::fclose(fp);

    int total = static_cast<int>(secs);
    int days = total / 86400;
    int hours = (total % 86400) / 3600;
    int mins = (total % 3600) / 60;

    char val[128];
    if (days > 0)
        std::snprintf(val, sizeof(val), "%d day%s, %d hour%s, %d min%s", days,
                      days == 1 ? "" : "s", hours, hours == 1 ? "" : "s", mins,
                      mins == 1 ? "" : "s");
    else if (hours > 0)
        std::snprintf(val, sizeof(val), "%d hour%s, %d min%s", hours,
                      hours == 1 ? "" : "s", mins, mins == 1 ? "" : "s");
    else
        std::snprintf(val, sizeof(val), "%d min%s", mins, mins == 1 ? "" : "s");

    add_info(ctx, "Uptime", "%s", val);
}

} // namespace rfxh::gather
