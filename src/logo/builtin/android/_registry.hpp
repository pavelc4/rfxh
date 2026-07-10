#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace rfxh::logo::builtin_android {

using LogoFunc = std::function<std::vector<std::string>()>;

inline const std::unordered_map<std::string, LogoFunc>& get_registry() {
    static const std::unordered_map<std::string, LogoFunc> registry = {
        {"grapheneos", builtin::android::logo_GrapheneOS},
        {"android", builtin::android::logo_android},
        {"android_small", builtin::android::logo_android_small},
    };
    return registry;
}

}
