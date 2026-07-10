#include "gather/gather.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace rfxh::gather {

static void read_gtk_setting(const char* key, char* out, int maxlen) {
    const char* home = std::getenv("HOME");
    if (!home)
        return;
    char path[512];
    std::snprintf(path, sizeof(path), "%s/.config/gtk-3.0/settings.ini", home);
    FILE* fp = std::fopen(path, "r");
    if (!fp)
        return;
    char buf[256];
    int keylen = static_cast<int>(std::strlen(key));
    while (std::fgets(buf, sizeof(buf), fp)) {
        if (std::strncmp(buf, key, keylen) == 0 && buf[keylen] == '=') {
            char* val = buf + keylen + 1;
            int len = std::strlen(val);
            while (len > 0 && (val[len - 1] == '\n' || val[len - 1] == '\r'))
                val[--len] = '\0';
            if (len > 0 && len < maxlen)
                std::memcpy(out, val, len + 1);
            break;
        }
    }
    std::fclose(fp);
}

void gather_theme(GatherContext& ctx) {
    char theme[64] = "";
    read_gtk_setting("gtk-theme-name", theme, sizeof(theme));
    if (theme[0])
        add_info(ctx, "Theme", "%s [GTK3]", theme);
}

void gather_icons(GatherContext& ctx) {
    char icons[64] = "";
    read_gtk_setting("gtk-icon-theme-name", icons, sizeof(icons));
    if (icons[0])
        add_info(ctx, "Icons", "%s [GTK3]", icons);
}

void gather_font(GatherContext& ctx) {
    char font[128] = "";
    read_gtk_setting("gtk-font-name", font, sizeof(font));
    if (font[0])
        add_info(ctx, "Font", "%s [GTK3]", font);
}

} // namespace rfxh::gather
