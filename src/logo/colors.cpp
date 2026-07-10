#include "logo/colors.hpp"
#include <cstring>
#include <strings.h>

namespace rfxh::logo {

DistroColors get_distro_colors(const char* distro) {
    DistroColors c;

    if (strcasecmp(distro, "gentoo") == 0) {
        c.outer = "\033[1;35m"; c.inner = "\033[1;37m";
    } else if (strcasecmp(distro, "arch") == 0) {
        c.outer = "\033[1;36m"; c.inner = "\033[1;36m";
    } else if (strcasecmp(distro, "ubuntu") == 0) {
        c.outer = "\033[1;31m"; c.inner = "\033[1;37m";
    } else if (strcasecmp(distro, "debian") == 0) {
        c.outer = "\033[1;31m"; c.inner = "\033[1;37m";
    } else if (strcasecmp(distro, "asahi") == 0 ||
               strcasecmp(distro, "asahi2") == 0 ||
               strcasecmp(distro, "fedora-asahi-remix") == 0) {
        c.outer = "\033[1;31m"; c.inner = "\033[1;37m";
    } else if (strcasecmp(distro, "fedora") == 0 ||
               strncasecmp(distro, "fedora-", 7) == 0) {
        c.outer = "\033[1;34m"; c.inner = "\033[1;37m";
    } else if (strcasecmp(distro, "nixos") == 0) {
        c.outer = "\033[1;34m"; c.inner = "\033[1;36m";
    } else if (strcasecmp(distro, "void") == 0) {
        c.outer = "\033[1;32m"; c.inner = "\033[1;32m";
    } else if (strcasecmp(distro, "alpine") == 0) {
        c.outer = "\033[1;34m"; c.inner = "\033[1;37m";
    } else if (strcasecmp(distro, "opensuse-tumbleweed") == 0 ||
               strcasecmp(distro, "opensuse-leap") == 0 ||
               strcasecmp(distro, "opensuse") == 0) {
        c.outer = "\033[1;32m"; c.inner = "\033[1;37m";
    }
    return c;
}

} // namespace rfxh::logo
