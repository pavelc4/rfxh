#include "platform/terminal.hpp"

#include <cstdio>
#include <cstdlib>
#include <sys/ioctl.h>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

namespace {

struct termios g_orig_termios;
bool g_termios_saved = false;
volatile sig_atomic_t g_term_resized = 0;

void restore_termios() {
    if (g_termios_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
        g_termios_saved = false;
    }
}

void handle_signal(int) {
    restore_termios();
    std::printf("\033[?25h");
    std::fflush(stdout);
    _exit(0);
}

void handle_winch(int) {
    g_term_resized = 1;
}

} // anonymous namespace

namespace rfxh::platform {

bool terminal_init() {
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &g_orig_termios) != 0)
        return false;
    g_termios_saved = true;

    raw = g_orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    // Install signal handlers
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);
    std::signal(SIGWINCH, handle_winch);

    return true;
}

void terminal_restore() {
    restore_termios();
}

void cursor_hide() {
    std::printf("\033[?25l");
    std::fflush(stdout);
}

void cursor_show() {
    std::printf("\033[?25h");
    std::fflush(stdout);
}

void screen_clear() {
    std::printf("\033[2J");
    std::fflush(stdout);
}

int terminal_rows() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0)
        return ws.ws_row;
    return 0;
}

int terminal_cols() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;
    return 0;
}

bool keypress_available() {
    struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
    int pret = poll(&pfd, 1, 0);
    if (pret > 0 && (pfd.revents & POLLIN)) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1)
            return true;
    }
    return false;
}

int keypress_read() {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1)
        return static_cast<unsigned char>(c);
    return 0;
}

void sleep_ms(int ms) {
    usleep(ms * 1000);
}

bool consume_resize() {
    if (g_term_resized) {
        g_term_resized = 0;
        return true;
    }
    return false;
}

} // namespace rfxh::platform
