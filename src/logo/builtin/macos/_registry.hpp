#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace rfxh::logo::builtin_macos {

using LogoFunc = std::function<std::vector<std::string>()>;

inline const std::unordered_map<std::string, LogoFunc>& get_registry() {
    static const std::unordered_map<std::string, LogoFunc> registry = {
        {"apple", builtin::macos::logo_Apple},
        {"apple_small", builtin::macos::logo_Apple_small},
        {"osx", builtin::macos::logo_OSX},
        {"macos", builtin::macos::logo_macOS},
        {"macos2", builtin::macos::logo_macOS2},
        {"macos2_small", builtin::macos::logo_macOS2_small},
        {"macos3", builtin::macos::logo_macOS3},
        {"macos_small", builtin::macos::logo_macOS_small},
    };
    return registry;
}

}
