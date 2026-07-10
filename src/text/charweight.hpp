#pragma once

namespace rfxh::text {

// Return visual weight (0.0-1.0) of a UTF-8 codepoint for heightmap generation.
float char_weight_utf8(const char* ch);

// Returns 1 if the ANSI escape is a cursor movement (not SGR color).
int is_cursor_escape(const char* p);

// Float comparator for qsort (ascending order).
int float_cmp(const void* a, const void* b);

} // namespace rfxh::text
