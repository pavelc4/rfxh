#pragma once

// Portable compatibility macros and functions

#ifdef _WIN32
    #define RFXH_STRCASECMP _stricmp
    #define RFXH_POPEN _popen
    #define RFXH_PCLOSE _pclose
    #define RFXH_SLEEP_MS(ms) Sleep(ms)
    // Windows doesn't have unistd.h
    #ifndef STDIN_FILENO
        #define STDIN_FILENO 0
    #endif
    #ifndef STDOUT_FILENO
        #define STDOUT_FILENO 1
    #endif
#else
    #define RFXH_STRCASECMP strcasecmp
    #define RFXH_POPEN popen
    #define RFXH_PCLOSE pclose
    #include <unistd.h>
    #define RFXH_SLEEP_MS(ms) usleep((ms) * 1000)
#endif
