#include "logo/logo.hpp"
#include "text/utf8.hpp"
#include <cstring>

namespace rfxh::logo {

static void process_logo_row(Logo& logo, int row) {
    const char* p = logo.data[row].data();
    int col = 0;
    int cur_color = 0;

    while (*p && col < kMaxLogoCols) {
        // Parse ANSI SGR escapes for foreground color
        if (p[0] == '\033' && p[1] == '[') {
            int i = 2;
            int num = 0;
            bool has_num = false;

            while (p[i] && ((p[i] >= '0' && p[i] <= '9') || p[i] == ';')) {
                if (p[i] >= '0' && p[i] <= '9') {
                    num = num * 10 + (p[i] - '0');
                    has_num = true;
                } else if (p[i] == ';') {
                    if (has_num && ((num >= 30 && num <= 37) || num == 39 ||
                                    (num >= 90 && num <= 97)))
                        cur_color = num;
                    if (has_num && (num == 0 || num == 22))
                        cur_color = 0;
                    num = 0;
                    has_num = false;
                }
                i++;
            }
            if (has_num && ((num >= 30 && num <= 37) || num == 39 ||
                            (num >= 90 && num <= 97)))
                cur_color = num;
            if (has_num && num == 0)
                cur_color = 0;
            if (p[i]) i++;
            if (cur_color > 0) logo.has_ansi = true;
            p += i;
            continue;
        }

        int len = text::utf8_char_len(static_cast<unsigned char>(*p));
        int actual = 0;
        while (actual < len && p[actual])
            actual++;

        std::memcpy(logo.cells[row][col].data(), p, actual);
        logo.cells[row][col][actual] = '\0';
        logo.cell_color[row][col] = cur_color;
        col++;
        p += actual;
    }

    logo.cell_counts[row] = col;
    if (col > logo.cols)
        logo.cols = col;
}

void process_logo(Logo& logo) {
    logo.cols = 0;
    for (int r = 0; r < logo.rows; r++)
        process_logo_row(logo, r);
}

} // namespace rfxh::logo
