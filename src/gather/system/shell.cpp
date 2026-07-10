#include "gather/gather.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace rfxh::gather {

void gather_shell(GatherContext& ctx) {
    const char* shell = std::getenv("SHELL");
    if (!shell)
        return;

    const char* name = std::strrchr(shell, '/');
    name = name ? name + 1 : shell;

    char version[128] = "";
    char cmd[256];
    std::snprintf(cmd, sizeof(cmd), "%s --version 2>/dev/null", shell);
    FILE* fp = popen(cmd, "r");
    if (fp) {
        char buf[256];
        if (std::fgets(buf, sizeof(buf), fp)) {
            const char* ver = std::strstr(buf, name);
            if (ver) {
                ver += std::strlen(name);
                while (*ver == ' ')
                    ver++;
            } else {
                ver = buf;
                while (*ver && !(*ver >= '0' && *ver <= '9'))
                    ver++;
            }
            if (*ver) {
                int len = 0;
                while (ver[len] && ver[len] != ' ' && ver[len] != '(' &&
                       ver[len] != '\n' && len < 30)
                    len++;
                std::memcpy(version, ver, len);
                version[len] = '\0';
            }
        }
        pclose(fp);
    }

    if (version[0])
        add_info(ctx, "Shell", "%s %s", name, version);
    else
        add_info(ctx, "Shell", "%s", name);
}

} // namespace rfxh::gather
