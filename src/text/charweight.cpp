#include "text/charweight.hpp"
#include <cstring>

namespace rfxh::text {

float char_weight_utf8(const char* ch) {
    // Single-byte ASCII
    if (static_cast<unsigned char>(ch[0]) < 0x80) {
        switch (ch[0]) {
        case 'M': return 1.00f;
        case 'N': return 0.88f;
        case 'm': return 0.76f;
        case 'd': return 0.66f;
        case 'h': return 0.56f;
        case 'b': return 0.56f;
        case 'y': return 0.46f;
        case 'o': return 0.38f;
        case 'n': return 0.38f;
        case 's': return 0.30f;
        case '+': return 0.22f;
        case ':': return 0.18f;
        case '=': return 0.22f;
        case '-': return 0.14f;
        case '`': return 0.08f;
        case '.': return 0.10f;
        case '/': return 0.12f;
        case '\'': return 0.06f;
        case ' ': return 0.0f;
        default:
            if (ch[0] >= 'A' && ch[0] <= 'Z') return 0.80f;
            if (ch[0] >= 'a' && ch[0] <= 'z') return 0.50f;
            if (ch[0] >= '0' && ch[0] <= '9') return 0.40f;
            return 0.15f;
        }
    }

    // Multi-byte UTF-8 block elements
    // Full block U+2588
    if (std::memcmp(ch, "\xe2\x96\x88", 3) == 0) return 1.00f;
    // Dark shade U+2593
    if (std::memcmp(ch, "\xe2\x96\x93", 3) == 0) return 0.75f;
    // Medium shade U+2592
    if (std::memcmp(ch, "\xe2\x96\x92", 3) == 0) return 0.50f;
    // Light shade U+2591
    if (std::memcmp(ch, "\xe2\x96\x91", 3) == 0) return 0.25f;

    // Half blocks (U+2580-258F)
    if (std::memcmp(ch, "\xe2\x96\x80", 3) == 0) return 0.50f; // upper
    if (std::memcmp(ch, "\xe2\x96\x84", 3) == 0) return 0.50f; // lower
    if (std::memcmp(ch, "\xe2\x96\x8c", 3) == 0) return 0.50f; // left
    if (std::memcmp(ch, "\xe2\x96\x90", 3) == 0) return 0.50f; // right

    // 3/4 blocks
    if (std::memcmp(ch, "\xe2\x96\x9b", 3) == 0) return 0.75f; // ▛
    if (std::memcmp(ch, "\xe2\x96\x9c", 3) == 0) return 0.75f; // ▜
    if (std::memcmp(ch, "\xe2\x96\x99", 3) == 0) return 0.75f; // ▙
    if (std::memcmp(ch, "\xe2\x96\x9f", 3) == 0) return 0.75f; // ▟

    // 1/4 blocks
    if (std::memcmp(ch, "\xe2\x96\x96", 3) == 0) return 0.25f; // ▖
    if (std::memcmp(ch, "\xe2\x96\x97", 3) == 0) return 0.25f; // ▗
    if (std::memcmp(ch, "\xe2\x96\x98", 3) == 0) return 0.25f; // ▘
    if (std::memcmp(ch, "\xe2\x96\x9d", 3) == 0) return 0.25f; // ▝

    // Box drawing chars (U+2500-257F)
    if (static_cast<unsigned char>(ch[0]) == 0xe2 &&
        (static_cast<unsigned char>(ch[1]) == 0x94 ||
         static_cast<unsigned char>(ch[1]) == 0x95))
        return 0.20f;

    // Braille (U+2800-28FF): weight by dot count
    if (static_cast<unsigned char>(ch[0]) == 0xe2 &&
        static_cast<unsigned char>(ch[1]) >= 0xa0 &&
        static_cast<unsigned char>(ch[1]) <= 0xa3) {
        unsigned char b = static_cast<unsigned char>(ch[2]);
        int dots = 0;
        while (b) { dots += b & 1; b >>= 1; }
        return dots / 8.0f;
    }

    return 0.30f;
}

int is_cursor_escape(const char* p) {
    if (p[0] != '\033' || p[1] != '[') return 0;
    int i = 2;
    while (p[i] && ((p[i] >= '0' && p[i] <= '9') || p[i] == ';'))
        i++;
    return (p[i] && p[i] != 'm');
}

int float_cmp(const void* a, const void* b) {
    float fa = *static_cast<const float*>(a);
    float fb = *static_cast<const float*>(b);
    return (fa > fb) - (fa < fb);
}

} // namespace rfxh::text
