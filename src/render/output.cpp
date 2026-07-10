#include "render/render.hpp"
#include "render/constants.hpp"
#include "logo/logo.hpp"
#include "text/shading.hpp"
#include "gather/gather.hpp"
#include "common/types.hpp"

#include <cstdio>
#include <cstring>
#include <string>

namespace rfxh::render {

void render_frame(const RenderEngine& eng, int render_height,
                  const gather::FetchLines& fetch_lines, int fetch_line_count,
                  int fetch_start, const logo::Logo& logo,
                  const std::string& color_inner, const std::string& color_outer,
                  bool use_color) {

    std::string buf;
    buf.reserve(render_height * 2048 + 64);

    buf += "\033[H";

    const char reset_seq[] = "\033[0m";
    const char clr_seq[]   = "\033[K";

    struct AnsiEntry { char s[8]; int len; };
    AnsiEntry ansi_tbl[128]{};
    int ansi_len[128]{};

    for (int i = 0; i < render_height; i++) {
        if (!use_color) {
            for (int j = 0; j < kFrameWidth; j++) {
                int ci = eng.shade_idx[i][j];
                if (ci < 0) {
                    buf += ' ';
                    continue;
                }
                const auto& sc = text::shading_char(ci);
                for (int k = 0; k < 4 && sc[k]; k++)
                    buf += sc[k];
            }
        } else {
            int prev_color = -1;
            for (int j = 0; j < kFrameWidth; j++) {
                int ci = eng.shade_idx[i][j];
                if (ci < 0) {
                    if (prev_color != -1) {
                        buf.append(reset_seq, 4);
                        prev_color = -1;
                    }
                    buf += ' ';
                } else {
                    int c = eng.colorbuf[i][j];
                    if (c != prev_color) {
                        if (logo.has_ansi && c > 0 && c < 128) {
                            if (ansi_len[c] == 0) {
                                ansi_len[c] = std::snprintf(ansi_tbl[c].s,
                                                            sizeof(ansi_tbl[c].s),
                                                            "\033[1;%dm", c);
                            }
                            buf.append(ansi_tbl[c].s, ansi_len[c]);
                        } else {
                            const std::string& cs = (c == 1) ? color_inner : color_outer;
                            buf.append(cs);
                        }
                        prev_color = c;
                    }
                    const auto& sc = text::shading_char(ci);
                    for (int k = 0; k < 4 && sc[k]; k++)
                        buf += sc[k];
                }
            }
            if (prev_color != -1) {
                buf.append(reset_seq, 4);
            }
        }

        int fi = i - fetch_start;
        if (fi >= 0 && fi < fetch_line_count) {
            buf.append(kGap, ' ');
            const char* fl = fetch_lines[fi].data();
            buf.append(fl, std::strlen(fl));
        }

        buf.append(clr_seq, 4);
        buf += '\n';
    }

    std::fwrite(buf.data(), 1, buf.size(), stdout);
    std::fflush(stdout);
}

} // namespace rfxh::render
