#pragma once

#include <cstdio>
#include <string>

namespace rfxh::platform {

// Portable popen/pclose wrapper
// On POSIX: uses popen/pclose
// On Windows: uses _popen/_pclose
FILE* pipe_open(const char* cmd, const char* mode);
int pipe_close(FILE* fp);

// Run a command and capture stdout as a string
std::string run_command(const char* cmd);

// Portable strcasecmp wrapper
// On POSIX: uses strcasecmp
// On Windows: uses _stricmp
int case_insensitive_cmp(const char* a, const char* b);

// Portable strncasecmp wrapper
// On POSIX: uses strncasecmp
// On Windows: uses _strnicmp
int case_insensitive_ncmp(const char* a, const char* b, size_t n);

} // namespace rfxh::platform
