#pragma once

#include "common/types.hpp"

namespace rfxh::text {

// Parse a UTF-8 shading string into individual codepoints.
// Populates the module-level shading_chars[] and shading_count.
void parse_shading(const char* str);

// Access parsed shading data (populated after parse_shading call).
int shading_count();
const Codepoint& shading_char(int index);

} // namespace rfxh::text
