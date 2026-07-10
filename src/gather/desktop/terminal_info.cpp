#include "gather/gather.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

namespace rfxh::gather {

void gather_terminal(GatherContext& ctx) {
    char term[64] = "";
    const char* tp = std::getenv("TERM_PROGRAM");
    if (tp && tp[0]) {
        std::strncpy(term, tp, sizeof(term) - 1);
    } else if (std::getenv("KITTY_WINDOW_ID")) {
        std::strcpy(term, "kitty");
    } else if (std::getenv("ALACRITTY_LOG")) {
        std::strcpy(term, "alacritty");
    } else if (std::getenv("WEZTERM_PANE")) {
        std::strcpy(term, "wezterm");
    } else if (std::getenv("GHOSTTY_RESOURCES_DIR")) {
        std::strcpy(term, "ghostty");
    } else if (std::getenv("TERMINAL_EMULATOR")) {
        std::strncpy(term, std::getenv("TERMINAL_EMULATOR"), sizeof(term) - 1);
    } else {
        pid_t pid = getpid();
        for (int depth = 0; depth < 4; depth++) {
            char path[64];
            std::snprintf(path, sizeof(path), "/proc/%d/status", pid);
            FILE* fp = std::fopen(path, "r");
            if (!fp) break;
            pid_t ppid = 0;
            char buf[128];
            while (std::fgets(buf, sizeof(buf), fp)) {
                if (std::sscanf(buf, "PPid:\t%d", &ppid) == 1)
                    break;
            }
            std::fclose(fp);
            if (ppid <= 0) break;
            pid = ppid;

            std::snprintf(path, sizeof(path), "/proc/%d/comm", pid);
            fp = std::fopen(path, "r");
            if (!fp) break;
            if (!std::fgets(term, sizeof(term), fp)) {
                std::fclose(fp);
                break;
            }
            std::fclose(fp);
            int len = std::strlen(term);
            while (len > 0 && (term[len - 1] == '\n' || term[len - 1] == '\r'))
                term[--len] = '\0';

            if (term[0] && std::strcmp(term, "bash") != 0 &&
                std::strcmp(term, "zsh") != 0 &&
                std::strcmp(term, "sh") != 0 &&
                std::strcmp(term, "dash") != 0 &&
                std::strcmp(term, "fish") != 0 &&
                std::strcmp(term, "nu") != 0 &&
                std::strcmp(term, "elvish") != 0 &&
                std::strcmp(term, "xonsh") != 0 &&
                std::strcmp(term, "tcsh") != 0 &&
                std::strcmp(term, "csh") != 0)
                break;
            term[0] = '\0';
        }
    }
    if (term[0])
        add_info(ctx, "Terminal", "%s", term);
}

} // namespace rfxh::gather
