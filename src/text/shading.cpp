#include "text/shading.hpp"
#include "text/utf8.hpp"
#include <cstring>

namespace rfxh::text {

static std::array<Codepoint, kMaxShading> s_shading_chars{};
static int s_shading_count = 0;

void parse_shading(const char* str) {
    s_shading_count = 0;
    const char* p = str;
    while (*p && s_shading_count < kMaxShading) {
        int len = utf8_char_len(static_cast<unsigned char>(*p));
        if (len > 4) len = 4;

        int actual = 0;
        while (actual < len && p[actual])
            actual++;

        std::memcpy(s_shading_chars[s_shading_count].data(), p, actual);
        s_shading_chars[s_shading_count][actual] = '\0';
        s_shading_count++;
        p += actual;
    }
    if (s_shading_count == 0) {
        std::strcpy(s_shading_chars[0].data(), ".");
        s_shading_count = 1;
    }
}

int shading_count() {
    return s_shading_count;
}

const Codepoint& shading_char(int index) {
    return s_shading_chars[index];
}

} // namespace rfxh::text
