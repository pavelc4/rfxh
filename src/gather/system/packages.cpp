#include "gather/gather.hpp"
#include <cstdio>
#include <cstring>
#include <dirent.h>

namespace rfxh::gather {

static int count_subdirs(const char* path) {
    DIR* d = opendir(path);
    if (!d)
        return 0;
    int count = 0;
    struct dirent* ent;
    while ((ent = readdir(d))) {
        if (ent->d_name[0] == '.')
            continue;
        count++;
    }
    closedir(d);
    return count;
}

static int count_emerge_pkgs() {
    DIR* d = opendir("/var/db/pkg");
    if (!d)
        return 0;
    int count = 0;
    struct dirent* cat;
    while ((cat = readdir(d))) {
        if (cat->d_name[0] == '.')
            continue;
        char path[256];
        std::snprintf(path, sizeof(path), "/var/db/pkg/%s", cat->d_name);
        count += count_subdirs(path);
    }
    closedir(d);
    return count;
}

static int count_file_lines(const char* path, const char* prefix) {
    FILE* fp = std::fopen(path, "r");
    if (!fp)
        return 0;
    int count = 0;
    char buf[512];
    int plen = prefix ? static_cast<int>(std::strlen(prefix)) : 0;
    while (std::fgets(buf, sizeof(buf), fp)) {
        if (!prefix || std::strncmp(buf, prefix, plen) == 0)
            count++;
    }
    std::fclose(fp);
    return count;
}

void gather_packages(GatherContext& ctx) {
    char val[128] = "";
    int n;

    n = count_emerge_pkgs();
    if (n > 0)
        std::snprintf(val, sizeof(val), "%d (emerge)", n);

    if (!val[0]) {
        n = count_subdirs("/var/lib/pacman/local");
        if (n > 1)
            std::snprintf(val, sizeof(val), "%d (pacman)", n - 1);
    }

    if (!val[0]) {
        n = count_file_lines("/var/lib/dpkg/status", "Package:");
        if (n > 0)
            std::snprintf(val, sizeof(val), "%d (dpkg)", n);
    }

    if (!val[0]) {
        n = count_subdirs("/var/db/xbps");
        if (n > 0)
            std::snprintf(val, sizeof(val), "%d (xbps)", n);
    }

    if (!val[0]) {
        n = count_file_lines("/lib/apk/db/installed", "P:");
        if (n > 0)
            std::snprintf(val, sizeof(val), "%d (apk)", n);
    }

    if (val[0])
        add_info(ctx, "Packages", "%s", val);
}

} // namespace rfxh::gather
