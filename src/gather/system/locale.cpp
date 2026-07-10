#include "gather/gather.hpp"
#include <cstdlib>

namespace rfxh::gather {

void gather_locale(GatherContext& ctx) {
    const char* lang = std::getenv("LANG");
    if (lang && lang[0])
        add_info(ctx, "Locale", "%s", lang);
}

} // namespace rfxh::gather
