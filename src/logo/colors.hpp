#pragma once

#include <string>

namespace rfxh::logo {

struct DistroColors {
    std::string outer = "\033[1;35m"; // bold magenta (default)
    std::string inner = "\033[1;37m"; // bold white (default)
};

DistroColors get_distro_colors(const char* distro);

} // namespace rfxh::logo
