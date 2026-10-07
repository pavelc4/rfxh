#include "platform/terminal.hpp"

#ifdef _WIN32

#include <windows.h>
#include <conio.h>
#include <cstdio>

namespace {

HANDLE g_hConsole = nullptr;
DWORD g_orig_mode = 0;
bool g_initialized = false;
int g_last_rows = 0;
int g_last_cols = 0;
int g_drag_dx = 0;
int g_drag_dy = 0;
int g_last_mx = -1;
int g_last_my = -1;
bool g_mdown = false;

void drain_console(bool& key_down) {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD record;
    DWORD events = 0;
    while (PeekConsoleInput(hInput, &record, 1, &events) && events > 0) {
        if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
            key_down = true;
            return; // leave key in queue for keypress_read
        }
        ReadConsoleInput(hInput, &record, 1, &events); // consume non-key
        if (record.EventType == MOUSE_EVENT) {
            const auto& m = record.Event.MouseEvent;
            int mx = static_cast<int>(m.dwMousePosition.X);
            int my = static_cast<int>(m.dwMousePosition.Y);
            bool down = (m.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
            if (down && g_mdown && g_last_mx >= 0) {
                g_drag_dx += mx - g_last_mx;
                g_drag_dy += my - g_last_my;
            }
            g_mdown = down;
            g_last_mx = mx;
            g_last_my = my;
        }
        // WINDOW_BUFFER_SIZE_EVENT handled via size poll in consume_resize
    }
}

} // anonymous namespace

namespace rfxh::platform {

bool terminal_init() {
    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_hConsole == INVALID_HANDLE_VALUE) return false;

    // Save original console mode
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hInput, &g_orig_mode);

    // Enable VT processing for ANSI escape codes
    DWORD mode = 0;
    GetConsoleMode(g_hConsole, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(g_hConsole, mode);

    // Set input to raw mode (no echo, no line buffering) + mouse/window events
    DWORD input_mode = 0;
    GetConsoleMode(hInput, &input_mode);
    input_mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
    input_mode |= ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT;
    SetConsoleMode(hInput, input_mode);

    g_initialized = true;
    g_last_rows = terminal_rows();
    g_last_cols = terminal_cols();
    return true;
}

void terminal_restore() {
    if (g_initialized) {
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        SetConsoleMode(hInput, g_orig_mode);
        g_initialized = false;
    }
}

void cursor_hide() {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 1;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(g_hConsole, &info);
}

void cursor_show() {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 25;
    info.bVisible = TRUE;
    SetConsoleCursorInfo(g_hConsole, &info);
}

void screen_clear() {
    // Use ANSI escape code (requires VT processing enabled)
    std::printf("\033[2J");
    std::fflush(stdout);
}

int terminal_rows() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(g_hConsole, &csbi))
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return 0;
}

int terminal_cols() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(g_hConsole, &csbi))
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return 0;
}

bool keypress_available() {
    bool key_down = false;
    drain_console(key_down);
    return key_down;
}

int keypress_read() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD record;
    DWORD events = 0;
    while (ReadConsoleInput(hInput, &record, 1, &events) && events > 0) {
        if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
            return static_cast<int>(record.Event.KeyEvent.uChar.AsciiChar);
        }
    }
    return 0;
}

void sleep_ms(int ms) {
    Sleep(ms);
}

bool consume_resize() {
    int r = terminal_rows();
    int c = terminal_cols();
    // drain any queued window/mouse events so the queue can't grow
    bool dummy = false;
    drain_console(dummy);
    if ((r > 0 && r != g_last_rows) || (c > 0 && c != g_last_cols)) {
        g_last_rows = r;
        g_last_cols = c;
        return true;
    }
    return false;
}

bool poll_mouse_drag(int& dx, int& dy) {
    bool dummy = false;
    drain_console(dummy);
    dx = g_drag_dx;
    dy = g_drag_dy;
    g_drag_dx = g_drag_dy = 0;
    return dx != 0 || dy != 0;
}

} // namespace rfxh::platform

#endif // _WIN32
