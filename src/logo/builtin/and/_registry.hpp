#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "logo_GrapheneOS.hpp"
#include "logo_android.hpp"
#include "logo_android_small.hpp"

namespace rfxh::logo::builtin_cat_android {

using LogoFunc = std::function<std::vector<std::string>()>;

inline const std::unordered_map<std::string, LogoFunc>& get_registry() {
    static const std::unordered_map<std::string, LogoFunc> registry = {
        {"grapheneos", builtin::cat_android::logo_GrapheneOS},
        {"android", builtin::cat_android::logo_android},
        {"android_small", builtin::cat_android::logo_android_small},
    };
    return registry;
}

}
