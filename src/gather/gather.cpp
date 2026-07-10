#include "gather/gather.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace rfxh::gather {

void add_line(GatherContext& ctx, const char* line) {
    if (ctx.line_count >= kMaxFetchLines) return;
    std::strncpy(ctx.lines[ctx.line_count].data(), line, kMaxLineLen - 1);
    ctx.lines[ctx.line_count][kMaxLineLen - 1] = '\0';
    ctx.line_count++;
}

void add_info(GatherContext& ctx, const char* label, const char* fmt, ...) {
    char val[kMaxLineLen];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(val, sizeof(val), fmt, ap);
    va_end(ap);

    char line[kMaxLineLen];
    std::snprintf(line, sizeof(line), "\033[1;%sm%s\033[0m: %s",
                  ctx.cfg.label_color.c_str(), label, val);

    // Refresh tick: replace in place
    if (ctx.cfg.is_refresh_pass && ctx.cfg.current_field >= 0 &&
        ctx.cfg.field_line[ctx.cfg.current_field] >= 0) {
        int idx = ctx.cfg.field_line[ctx.cfg.current_field];
        std::strncpy(ctx.lines[idx].data(), line, kMaxLineLen - 1);
        ctx.lines[idx][kMaxLineLen - 1] = '\0';
        return;
    }
    if (ctx.cfg.current_field >= 0)
        ctx.cfg.field_line[ctx.cfg.current_field] = ctx.line_count;
    add_line(ctx, line);
}

void gather_title(GatherContext& ctx) {
    char user[64] = "";
    char host[64] = "";

    char* login = getlogin();
    if (login)
        std::strncpy(user, login, sizeof(user) - 1);
    else {
        const char* env = std::getenv("USER");
        if (env) std::strncpy(user, env, sizeof(user) - 1);
    }
    gethostname(host, sizeof(host));

    char line[kMaxLineLen];
    std::snprintf(line, sizeof(line),
                  "\033[1;%sm%s\033[0m@\033[1;%sm%s\033[0m",
                  ctx.cfg.label_color.c_str(), user,
                  ctx.cfg.label_color.c_str(), host);
    add_line(ctx, line);

    // Separator line
    int title_len = static_cast<int>(std::strlen(user)) + 1 +
                    static_cast<int>(std::strlen(host));
    int sep_len = static_cast<int>(ctx.cfg.config_separator.size());
    if (sep_len == 0) sep_len = 1;

    char sep[kMaxLineLen];
    int pos = 0;
    for (int i = 0; i < title_len && pos + sep_len < kMaxLineLen; i++) {
        std::memcpy(sep + pos, ctx.cfg.config_separator.c_str(), sep_len);
        pos += sep_len;
    }
    sep[pos] = '\0';
    add_line(ctx, sep);
}

void gather_colors(GatherContext& ctx) {
    // Two rows of 4 color blocks
    add_line(ctx, "\033[40m   \033[0m \033[41m   \033[0m \033[42m   \033[0m \033[43m   \033[0m");
    add_line(ctx, "\033[44m   \033[0m \033[45m   \033[0m \033[46m   \033[0m \033[47m   \033[0m");
}

} // namespace rfxh::gather
