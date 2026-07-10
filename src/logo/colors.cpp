#include "logo/colors.hpp"
#include "platform/process.hpp"
#include <cstring>

namespace rfxh::logo {

DistroColors get_distro_colors(const char* distro) {
    DistroColors c;

    if (platform::case_insensitive_cmp(distro, "gentoo") == 0) {
        c.outer = "\033[1;35m"; c.inner = "\033[1;37m";
    } else if (platform::case_insensitive_cmp(distro, "arch") == 0) {
        c.outer = "\033[1;36m"; c.inner = "\033[1;36m";
    } else if (platform::case_insensitive_cmp(distro, "ubuntu") == 0) {
        c.outer = "\033[1;31m"; c.inner = "\033[1;37m";
    } else if (platform::case_insensitive_cmp(distro, "debian") == 0) {
        c.outer = "\033[1;31m"; c.inner = "\033[1;37m";
    } else if (platform::case_insensitive_cmp(distro, "asahi") == 0 ||
               platform::case_insensitive_cmp(distro, "asahi2") == 0 ||
               platform::case_insensitive_cmp(distro, "fedora-asahi-remix") == 0) {
        c.outer = "\033[1;31m"; c.inner = "\033[1;37m";
    } else if (platform::case_insensitive_cmp(distro, "fedora") == 0 ||
               strncasecmp(distro, "fedora-", 7) == 0) {
        c.outer = "\033[1;34m"; c.inner = "\033[1;37m";
    } else if (platform::case_insensitive_cmp(distro, "nixos") == 0) {
        c.outer = "\033[1;34m"; c.inner = "\033[1;36m";
    } else if (platform::case_insensitive_cmp(distro, "void") == 0) {
        c.outer = "\033[1;32m"; c.inner = "\033[1;32m";
    } else if (platform::case_insensitive_cmp(distro, "alpine") == 0) {
        c.outer = "\033[1;34m"; c.inner = "\033[1;37m";
    } else if (platform::case_insensitive_cmp(distro, "opensuse-tumbleweed") == 0 ||
               platform::case_insensitive_cmp(distro, "opensuse-leap") == 0 ||
               platform::case_insensitive_cmp(distro, "opensuse") == 0) {
        c.outer = "\033[1;32m"; c.inner = "\033[1;37m";
    } else if (strncasecmp(distro, "android", 7) == 0) {
        c.outer = "\033[1;32m"; c.inner = "\033[1;37m";
    } else if (strncasecmp(distro, "lineage", 7) == 0) {
        c.outer = "\033[1;36m"; c.inner = "\033[1;37m";
    } else if (platform::case_insensitive_cmp(distro, "grapheneos") == 0 ||
               platform::case_insensitive_cmp(distro, "bugdroid") == 0) {
        c.outer = "\033[1;32m"; c.inner = "\033[1;37m";
    }
    return c;
}

} // namespace rfxh::logo
