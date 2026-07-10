#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "logo_Apple.hpp"
#include "logo_Apple_small.hpp"
#include "logo_OSX.hpp"
#include "logo_macOS.hpp"
#include "logo_macOS2.hpp"
#include "logo_macOS2_small.hpp"
#include "logo_macOS3.hpp"
#include "logo_macOS_small.hpp"

namespace rfxh::logo::builtin_cat_macos {

using LogoFunc = std::function<std::vector<std::string>()>;

inline const std::unordered_map<std::string, LogoFunc>& get_registry() {
    static const std::unordered_map<std::string, LogoFunc> registry = {
        {"apple", builtin::cat_macos::logo_Apple},
        {"apple_small", builtin::cat_macos::logo_Apple_small},
        {"osx", builtin::cat_macos::logo_OSX},
        {"macos", builtin::cat_macos::logo_macOS},
        {"macos2", builtin::cat_macos::logo_macOS2},
        {"macos2_small", builtin::cat_macos::logo_macOS2_small},
        {"macos3", builtin::cat_macos::logo_macOS3},
        {"macos_small", builtin::cat_macos::logo_macOS_small},
    };
    return registry;
}

}
