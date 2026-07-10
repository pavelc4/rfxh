#include "terminal/terminal.hpp"

#include <cstdio>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>

static struct termios orig_termios;
static int termios_saved = 0;

static void cleanup(void) {
    if (termios_saved)
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    std::printf("\033[?25h");
    std::fflush(stdout);
}

namespace rfxh::terminal {

    RawModeGuard::RawModeGuard() {
        struct termios raw;
        tcgetattr(STDIN_FILENO, &orig_termios);
        termios_saved = 1;
        std::atexit(cleanup);

        raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        raw.c_iflag &= ~(IXON | ICRNL);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

        action_ = true;
        std::printf("\033[?25l");
        std::fflush(stdout);
    }

    RawModeGuard::~RawModeGuard() {
        cleanup();
        action_ = false;
    }

} // namespace rfxh::terminal
