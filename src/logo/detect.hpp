#pragma once

#include <string>

namespace rfxh::logo {

struct DistroInfo {
    std::string id;
    std::string id_like;
};

// Detect distro ID via fastfetch JSON or /etc/os-release
DistroInfo detect_distro();

} // namespace rfxh::logo
