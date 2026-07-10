#include "gather/gather.hpp"
#include <cstdio>
#include <cstring>

namespace rfxh::gather {

void gather_host(GatherContext& ctx) {
    char model[128] = "";

    FILE* fp = std::fopen("/proc/device-tree/model", "r");
    if (!fp)
        fp = std::fopen("/sys/class/dmi/id/product_name", "r");
    if (fp) {
        if (std::fgets(model, sizeof(model), fp)) {
            int len = std::strlen(model);
            while (len > 0 && (model[len - 1] == '\n' || model[len - 1] == '\r' ||
                               model[len - 1] == '\0'))
                len--;
            model[len] = '\0';
        }
        std::fclose(fp);
    }

    if (model[0])
        add_info(ctx, "Host", "%s", model);
}

} // namespace rfxh::gather
