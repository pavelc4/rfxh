#include "platform/process.hpp"

#ifdef _WIN32

#include <cstdio>
#include <cstring>
#include <string>

namespace rfxh::platform {

FILE* pipe_open(const char* cmd, const char* mode) {
    return _popen(cmd, mode);
}

int pipe_close(FILE* fp) {
    return _pclose(fp);
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
    return _stricmp(a, b);
}

} // namespace rfxh::platform

#endif // _WIN32
