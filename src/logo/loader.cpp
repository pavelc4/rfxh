#include "logo/logo.hpp"
#include "logo/library.hpp"
#include "platform/process.hpp"
#include "text/charweight.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Include generated logo registry (split across categories)
#include "builtin/_registry.hpp"

namespace rfxh::logo {

static bool is_cursor_escape(const char* p) {
    if (p[0] != '\033' || p[1] != '[') return false;
    int i = 2;
    while (p[i] && ((p[i] >= '0' && p[i] <= '9') || p[i] == ';'))
        i++;
    return (p[i] && p[i] != 'm');
}

static void trim_newlines(char* buf) {
    int len = static_cast<int>(std::strlen(buf));
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
        buf[--len] = '\0';
}

static void strip_trailing_empty(Logo& logo) {
    while (logo.rows > 0 && logo.data[logo.rows - 1][0] == '\0')
        logo.rows--;
}

bool load_logo_file(Logo& logo) {
    const char* home = std::getenv("HOME");
    if (!home) return false;

    char path[512];
    std::snprintf(path, sizeof(path), "%s/.config/fetch/logo.txt", home);
    FILE* fp = std::fopen(path, "r");
    if (!fp) return false;

    char buf[512];
    while (logo.rows < kMaxLogoRows && std::fgets(buf, sizeof(buf), fp)) {
        trim_newlines(buf);
        int len = static_cast<int>(std::strlen(buf));

        // Extract "# distro:" header from first line
        if (logo.rows == 0 && std::strncmp(buf, "# distro:", 9) == 0) {
            const char* val = buf + 9;
            while (*val == ' ') val++;
            logo.file_distro = val;
            continue;
        }
        if (len == 0 && logo.rows == 0) continue;

        std::memcpy(logo.data[logo.rows].data(), buf, len + 1);
        logo.rows++;
    }
    std::fclose(fp);
    strip_trailing_empty(logo);
    return logo.rows > 0;
}

static bool load_logo_ff_colored(Logo& logo, const char* name) {
    char cmd[256];
    std::snprintf(cmd, sizeof(cmd),
                  "fastfetch -l %s -s break --pipe false 2>/dev/null", name);
    FILE* fp = popen(cmd, "r");
    if (!fp) return false;

    char buf[512];
    while (logo.rows < kMaxLogoRows && std::fgets(buf, sizeof(buf), fp)) {
        trim_newlines(buf);
        int len = static_cast<int>(std::strlen(buf));

        // Truncate at cursor movement sequences, keeping last SGR
        int last_sgr_end = -1;
        for (int i = 0; i < len - 2; i++) {
            if (is_cursor_escape(&buf[i])) {
                int cut = (last_sgr_end >= 0) ? last_sgr_end : i;
                buf[cut] = '\0';
                len = cut;
                break;
            }
            if (buf[i] == '\033' && buf[i+1] == '[') {
                int j = i + 2;
                while (buf[j] && ((buf[j] >= '0' && buf[j] <= '9') || buf[j] == ';'))
                    j++;
                if (buf[j] == 'm') {
                    last_sgr_end = j + 1;
                    i = j;
                }
            }
        }

        if (len == 0 && logo.rows == 0) continue;
        if (len == 0) break;

        std::memcpy(logo.data[logo.rows].data(), buf, len + 1);
        logo.rows++;
    }
    pclose(fp);
    strip_trailing_empty(logo);
    return logo.rows > 0;
}

static bool load_logo_ff_plain(Logo& logo, const char* name) {
    FILE* fp = popen("fastfetch --print-logos 2>/dev/null", "r");
    if (!fp) return false;

    char buf[512];
    bool found = false;
    int name_len = static_cast<int>(std::strlen(name));

    while (std::fgets(buf, sizeof(buf), fp)) {
        trim_newlines(buf);
        int len = static_cast<int>(std::strlen(buf));

        if (!found) {
            // Look for "name:" header
            if (len > 0 && len <= name_len + 1 && buf[len-1] == ':') {
                buf[len-1] = '\0';
                if (platform::case_insensitive_cmp(buf, name) == 0)
                    found = true;
            }
            continue;
        }

        // Detect next logo header (stops collecting)
        if (len > 1 && len < 40 && buf[len-1] == ':' && logo.rows > 0 &&
            ((buf[0] >= 'A' && buf[0] <= 'Z') || (buf[0] >= 'a' && buf[0] <= 'z'))) {
            bool is_header = true;
            for (int i = 0; i < len; i++) {
                if (buf[i] == '\033') { is_header = false; break; }
            }
            if (is_header) break;
        }

        if (logo.rows >= kMaxLogoRows) break;
        std::memcpy(logo.data[logo.rows].data(), buf, len + 1);
        logo.rows++;
    }
    pclose(fp);
    strip_trailing_empty(logo);
    return logo.rows > 0;
}

bool load_logo_fastfetch(Logo& logo, const char* name) {
    if (load_logo_ff_colored(logo, name))
        return true;
    return load_logo_ff_plain(logo, name);
}

bool load_logo_library(Logo& logo, const char* name) {
    auto* lines = get_builtin_logo(name);
    if (lines && !lines->empty()) {
        for (const auto& line : *lines) {
            if (logo.rows >= kMaxLogoRows) break;
            int len = static_cast<int>(line.size());
            std::memcpy(logo.data[logo.rows].data(), line.c_str(), len + 1);
            logo.rows++;
        }
        return logo.rows > 0;
    }
    return false;
}

void load_default_logo(Logo& logo) {
    static const char* gentoo[] = {
        "         -/oyddmdhs+:.            ",
        "     -odNMMMMMMMMNNmhy+-`         ",
        "   -yNMMMMMMMMMMMNNNmmdhy+-       ",
        " `omMMMMMMMMMMMMNmdmmmmddhhy/`    ",
        " omMMMMMMMMMMMNhhyyyohmdddhhhdo`  ",
        ".ydMMMMMMMMMMdhs++so/smdddhhhhdm+`",
        " oyhdmNMMMMMMMNdyooydMddddhhhhyhNd.",
        "  :oyhhdNNMMMMMMMNNMMMdddhhhhhyymMh",
        "    .:+sydNMMMMMNNMMMMdddhhhhhhmMmy",
        "       /mMMMMMMNNNMMMdddhhhhhmMNhs:",
        "    `oNMMMMMMMNNNMMMddddhhdmMNhs+` ",
        "  `sNMMMMMMMMNNNMMMdddddmNMmhs/.   ",
        " /NMMMMMMMMNNNNMMMdddmNMNdso:`     ",
        "+MMMMMMMNNNNNMMMMdMNMNdso/-        ",
        "yMMNNNNNNNMMMMMNNMmhs+/-`          ",
        "/hMMNNNNNNNNMNdhs++/-`             ",
        "`/ohdmmddhys+++/:.`                ",
        "  `-//////:--.                     ",
    };
    logo.rows = 18;
    for (int i = 0; i < logo.rows; i++) {
        int len = static_cast<int>(std::strlen(gentoo[i]));
        std::memcpy(logo.data[i].data(), gentoo[i], len + 1);
    }
}

} // namespace rfxh::logo
