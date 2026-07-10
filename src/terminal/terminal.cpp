#include "terminal/terminal.hpp"

#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <termios.h>
#include <unistd.h>

namespace {

    struct termios orig_termios;
    bool termios_saved = false;

    rfxh::terminal::RawModeGuard* g_active_guard = nullptr;
    volatile sig_atomic_t g_term_resized = 0;

    void restore_termios() {
        if (termios_saved) {
            tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
            termios_saved = false;
        }
    }

    void handle_signal(int) {
        if (g_active_guard)
            g_active_guard->restore();
        else
            restore_termios();
        std::printf("\033[?25h");
        std::fflush(stdout);
        _exit(0);
    }

    void handle_winch(int) {
        g_term_resized = 1;
    }

} // anonymous namespace

namespace rfxh::terminal {

    RawModeGuard::RawModeGuard() {
        struct termios raw;
        tcgetattr(STDIN_FILENO, &orig_termios);
        termios_saved = true;
        g_active_guard = this;

        raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        raw.c_iflag &= ~(IXON | ICRNL);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

        active_ = true;
        std::printf("\033[?25l");
        std::fflush(stdout);
    }

    RawModeGuard::~RawModeGuard() {
        restore();
    }

    void RawModeGuard::restore() {
        if (active_) {
            restore_termios();
            g_active_guard = nullptr;
            active_ = false;
        }
        std::printf("\033[?25h");
        std::fflush(stdout);
    }

    void install_signal_handlers() {
        std::signal(SIGINT, handle_signal);
        std::signal(SIGTERM, handle_signal);
        std::signal(SIGWINCH, handle_winch);
    }

    bool consume_resize_flag() {
        if (g_term_resized) {
            g_term_resized = 0;
            return true;
        }
        return false;
    }

} // namespace rfxh::terminal
