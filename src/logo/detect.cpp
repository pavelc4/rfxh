#include "logo/detect.hpp"
#include <cstdio>
#include <cstring>

namespace rfxh::logo {

static bool parse_os_release_val(const char* buf, int prefix_len, std::string& out) {
    int len = static_cast<int>(std::strlen(buf));
    if (len - prefix_len <= 0) return false;

    // Skip to value, strip quotes and newlines
    const char* val = buf + prefix_len;
    while (*val == '"') val++;

    char tmp[256];
    int vlen = static_cast<int>(std::strlen(val));
    if (vlen >= static_cast<int>(sizeof(tmp))) return false;
    std::memcpy(tmp, val, vlen + 1);

    // Trim trailing quote and whitespace
    while (vlen > 0 && (tmp[vlen-1] == '\n' || tmp[vlen-1] == '\r' || tmp[vlen-1] == '"'))
        tmp[--vlen] = '\0';

    if (vlen > 0) {
        out = tmp;
        return true;
    }
    return false;
}

static bool detect_fastfetch(DistroInfo& info) {
    FILE* fp = popen("fastfetch --json 2>/dev/null", "r");
    if (!fp) return false;

    char buf[1024];
    bool found_os = false;

    while (std::fgets(buf, sizeof(buf), fp)) {
        if (std::strstr(buf, "\"OS\""))
            found_os = true;

        if (found_os) {
            // Extract "id": "..."
            char* id_pos = std::strstr(buf, "\"id\"");
            if (id_pos) {
                char* colon = std::strchr(id_pos, ':');
                if (colon) {
                    char* q1 = std::strchr(colon, '"');
                    if (q1) {
                        q1++;
                        char* q2 = std::strchr(q1, '"');
                        if (q2 && q2 - q1 > 0) {
                            info.id.assign(q1, q2 - q1);
                            // Also look for idLike
                            char* like_pos = std::strstr(buf, "\"idLike\"");
                            if (like_pos) {
                                char* lcolon = std::strchr(like_pos, ':');
                                if (lcolon) {
                                    char* lq1 = std::strchr(lcolon, '"');
                                    if (lq1) {
                                        lq1++;
                                        char* lq2 = std::strchr(lq1, '"');
                                        if (lq2 && lq2 - lq1 > 0)
                                            info.id_like.assign(lq1, lq2 - lq1);
                                    }
                                }
                            }
                            pclose(fp);
                            return true;
                        }
                    }
                }
            }
        }
    }
    pclose(fp);
    return false;
}

static bool detect_os_release(DistroInfo& info) {
    FILE* fp = std::fopen("/etc/os-release", "r");
    if (!fp) return false;

    char buf[256];
    bool found_id = false;

    while (std::fgets(buf, sizeof(buf), fp)) {
        if (!found_id && std::strncmp(buf, "ID=", 3) == 0)
            found_id = parse_os_release_val(buf, 3, info.id);
        else if (std::strncmp(buf, "ID_LIKE=", 8) == 0)
            parse_os_release_val(buf, 8, info.id_like);
    }
    std::fclose(fp);
    return found_id;
}

DistroInfo detect_distro() {
    DistroInfo info;
    if (detect_fastfetch(info))
        return info;
    detect_os_release(info);
    return info;
}

} // namespace rfxh::logo
