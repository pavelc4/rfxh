#include "gather/gather.hpp"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>

namespace rfxh::gather {

void gather_wm(GatherContext& ctx) {
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    const char* session = std::getenv("XDG_SESSION_TYPE");
    const char* desktop = std::getenv("XDG_CURRENT_DESKTOP");
    bool is_wayland =
        (wayland && wayland[0]) || (session && std::strcmp(session, "wayland") == 0);

    char wm[64] = "";

    const char* hyprland = std::getenv("HYPRLAND_INSTANCE_SIGNATURE");
    if (hyprland)
        std::strcpy(wm, "Hyprland");

    if (!wm[0]) {
        static const char* known_wms[] = {"dwl",   "sway",    "river", "labwc",
                                          "weston", "i3",      "bspwm", "openbox",
                                          "awesome", "dwm",   nullptr};
        DIR* proc = opendir("/proc");
        if (proc) {
            struct dirent* ent;
            while ((ent = readdir(proc)) && !wm[0]) {
                if (ent->d_name[0] < '1' || ent->d_name[0] > '9')
                    continue;
                char path[64];
                std::snprintf(path, sizeof(path), "/proc/%s/comm", ent->d_name);
                FILE* fp = std::fopen(path, "r");
                if (!fp)
                    continue;
                char comm[64] = "";
                if (std::fgets(comm, sizeof(comm), fp)) {
                    int len = std::strlen(comm);
                    while (len > 0 && (comm[len - 1] == '\n' || comm[len - 1] == '\r'))
                        comm[--len] = '\0';
                    for (int i = 0; known_wms[i]; i++) {
                        if (std::strcmp(comm, known_wms[i]) == 0) {
                            std::strncpy(wm, comm, sizeof(wm) - 1);
                            break;
                        }
                    }
                }
                std::fclose(fp);
            }
            closedir(proc);
        }
    }

    if (!wm[0] && desktop && desktop[0]) {
        char first[32];
        int n = 0;
        while (desktop[n] && desktop[n] != ':' && n < static_cast<int>(sizeof(first)) - 1) {
            first[n] = desktop[n];
            n++;
        }
        first[n] = '\0';

        auto strcasecmp_ = [](const char* a, const char* b) -> int {
            while (*a && *b) {
                int ca = std::toupper(static_cast<unsigned char>(*a));
                int cb = std::toupper(static_cast<unsigned char>(*b));
                if (ca != cb) return ca - cb;
                a++; b++;
            }
            return static_cast<int>(*a) - static_cast<int>(*b);
        };

        if (strcasecmp_(first, "KDE") == 0)
            std::strncpy(wm, "KWin", sizeof(wm) - 1);
        else if (strcasecmp_(first, "GNOME") == 0)
            std::strncpy(wm, "Mutter", sizeof(wm) - 1);
        else if (strcasecmp_(first, "XFCE") == 0)
            std::strncpy(wm, "xfwm4", sizeof(wm) - 1);
        else if (strcasecmp_(first, "Cinnamon") == 0)
            std::strncpy(wm, "Muffin", sizeof(wm) - 1);
        else if (strcasecmp_(first, "MATE") == 0)
            std::strncpy(wm, "Marco", sizeof(wm) - 1);
        else if (strcasecmp_(first, "LXQt") == 0)
            std::strncpy(wm, "Openbox", sizeof(wm) - 1);
        else if (strcasecmp_(first, "Budgie") == 0)
            std::strncpy(wm, "Mutter", sizeof(wm) - 1);
        else if (strcasecmp_(first, "Deepin") == 0)
            std::strncpy(wm, "KWin", sizeof(wm) - 1);
        else
            std::strncpy(wm, desktop, sizeof(wm) - 1);
    }

    if (wm[0])
        add_info(ctx, "WM", "%s%s", wm, is_wayland ? " (Wayland)" : "");
}

} // namespace rfxh::gather
