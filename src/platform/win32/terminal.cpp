#include "platform/terminal.hpp"

#ifdef _WIN32

#include <windows.h>
#include <conio.h>
#include <cstdio>

namespace {

HANDLE g_hConsole = nullptr;
DWORD g_orig_mode = 0;
bool g_initialized = false;

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

    // Set input to raw mode (no echo, no line buffering)
    DWORD input_mode = 0;
    GetConsoleMode(hInput, &input_mode);
    input_mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
    SetConsoleMode(hInput, input_mode);

    g_initialized = true;
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
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD record;
    DWORD events = 0;
    while (PeekConsoleInput(hInput, &record, 1, &events) && events > 0) {
        if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
            return true;
        }
        ReadConsoleInput(hInput, &record, 1, &events);
    }
    return false;
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
    // Windows console resize doesn't send signals like SIGWINCH
    // Could poll terminal size changes but skip for now
    return false;
}

} // namespace rfxh::platform

#endif // _WIN32
