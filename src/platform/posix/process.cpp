#include "platform/process.hpp"

#include <cstring>
#include <string>
#ifdef _WIN32
    #include <cstdio>
    #define popen _popen
    #define pclose _pclose
    #define strcasecmp _stricmp
#else
    #include <cstdio>
    #include <strings.h>
#endif

namespace rfxh::platform {

FILE* pipe_open(const char* cmd, const char* mode) {
    return popen(cmd, mode);
}

int pipe_close(FILE* fp) {
    return pclose(fp);
}

std::string run_command(const char* cmd) {
    FILE* fp = pipe_open(cmd, "r");
    if (!fp) return "";

    std::string result;
    char buf[512];
    while (std::fgets(buf, sizeof(buf), fp)) {
        result += buf;
    }
    pipe_close(fp);

    // Trim trailing newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        result.pop_back();

    return result;
}

int case_insensitive_cmp(const char* a, const char* b) {
    return strcasecmp(a, b);
}

} // namespace rfxh::platform
