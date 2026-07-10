#include "gather/gather.hpp"
#include <cstdio>
#include <cstring>
#include <dirent.h>

namespace rfxh::gather {

void gather_display(GatherContext& ctx) {
    DIR* d = opendir("/sys/class/drm");
    if (!d)
        return;
    struct dirent* ent;
    int emitted = 0;
    while ((ent = readdir(d))) {
        if (std::strncmp(ent->d_name, "card", 4) != 0)
            continue;
        const char* dash = std::strchr(ent->d_name + 4, '-');
        if (!dash)
            continue;

        char path[256];
        std::snprintf(path, sizeof(path), "/sys/class/drm/%s/status", ent->d_name);
        FILE* fp = std::fopen(path, "r");
        if (!fp)
            continue;
        char status[32] = "";
        if (std::fgets(status, sizeof(status), fp)) {
            int l = std::strlen(status);
            while (l > 0 && (status[l - 1] == '\n' || status[l - 1] == '\r'))
                status[--l] = '\0';
        }
        std::fclose(fp);
        if (std::strcmp(status, "connected") != 0)
            continue;

        std::snprintf(path, sizeof(path), "/sys/class/drm/%s/modes", ent->d_name);
        fp = std::fopen(path, "r");
        if (!fp)
            continue;
        char mode[32] = "";
        if (std::fgets(mode, sizeof(mode), fp)) {
            int l = std::strlen(mode);
            while (l > 0 && (mode[l - 1] == '\n' || mode[l - 1] == '\r'))
                mode[--l] = '\0';
        }
        std::fclose(fp);
        if (!mode[0])
            continue;

        add_info(ctx, "Display", "%s @ %s", dash + 1, mode);
        emitted++;
    }
    closedir(d);

    if (!emitted) {
        d = opendir("/sys/class/drm");
        if (!d) return;
        while ((ent = readdir(d))) {
            if (std::strncmp(ent->d_name, "card", 4) != 0)
                continue;
            char path[256];
            std::snprintf(path, sizeof(path), "/sys/class/drm/%s/modes", ent->d_name);
            FILE* fp = std::fopen(path, "r");
            if (!fp)
                continue;
            char mode[32] = "";
            if (std::fgets(mode, sizeof(mode), fp)) {
                int l = std::strlen(mode);
                while (l > 0 && (mode[l - 1] == '\n' || mode[l - 1] == '\r'))
                    mode[--l] = '\0';
            }
            std::fclose(fp);
            if (mode[0]) {
                add_info(ctx, "Display", "%s", mode);
                break;
            }
        }
        closedir(d);
    }
}

} // namespace rfxh::gather
