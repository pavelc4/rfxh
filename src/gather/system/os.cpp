#include "gather/gather.hpp"
#include <cstdio>
#include <cstring>
#include <sys/utsname.h>

namespace rfxh::gather {

void gather_os(GatherContext& ctx) {
    std::string pretty;

    FILE* fp = std::fopen("/etc/os-release", "r");
    if (fp) {
        char buf[256];
        while (std::fgets(buf, sizeof(buf), fp)) {
            if (std::strncmp(buf, "PRETTY_NAME=", 12) == 0) {
                char* val = buf + 12;
                int len = std::strlen(val);
                while (len > 0 && (val[len - 1] == '\n' || val[len - 1] == '\r'))
                    val[--len] = '\0';
                if (*val == '\'' || *val == '"')
                    val++;
                len = std::strlen(val);
                if (len > 0 && (val[len - 1] == '\'' || val[len - 1] == '"'))
                    val[--len] = '\0';
                pretty = val;
                break;
            }
        }
        std::fclose(fp);
    }

    if (pretty.empty())
        pretty = "Linux";

    struct utsname u;
    uname(&u);

    add_info(ctx, "OS", "%s %s", pretty.c_str(), u.machine);
}

} // namespace rfxh::gather
