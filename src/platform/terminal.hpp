#pragma once

namespace rfxh::platform {

// Initialize terminal for animation (raw mode on POSIX, VT processing on Windows)
bool terminal_init();

// Restore terminal to original state
void terminal_restore();

// Hide cursor
void cursor_hide();

// Show cursor
void cursor_show();

// Clear screen
void screen_clear();

// Get terminal rows (returns 0 on failure)
int terminal_rows();

// Get terminal columns (returns 0 on failure)
int terminal_cols();

// Check if a keypress is available (non-blocking)
bool keypress_available();

// Read a single keypress (returns 0 if none available)
int keypress_read();

// Sleep for milliseconds
void sleep_ms(int ms);

// Check if terminal was resized (consumes flag)
bool consume_resize();

} // namespace rfxh::platform
