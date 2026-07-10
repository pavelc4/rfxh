#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "logo_GrapheneOS.hpp"
#include "logo_an_android_12.hpp"
#include "logo_an_android_13.hpp"
#include "logo_an_android_14.hpp"
#include "logo_an_android_15.hpp"
#include "logo_an_android_16.hpp"
#include "logo_an_android_17.hpp"
#include "logo_android.hpp"
#include "logo_android_small.hpp"
#include "logo_an_robot.hpp"
#include "logo_lineage_os.hpp"

namespace rfxh::logo::builtin::cat_android {

using LogoFunc = std::function<std::vector<std::string>()>;

inline const std::unordered_map<std::string, LogoFunc>& get_registry() {
    static const std::unordered_map<std::string, LogoFunc> registry = {
        {"android 12", logo_an_android_12},
        {"android_12", logo_an_android_12},
        {"android 13", logo_an_android_13},
        {"android_13", logo_an_android_13},
        {"android 14", logo_an_android_14},
        {"android_14", logo_an_android_14},
        {"android 15", logo_an_android_15},
        {"android_15", logo_an_android_15},
        {"android 16", logo_an_android_16},
        {"android_16", logo_an_android_16},
        {"android 17", logo_an_android_17},
        {"android_17", logo_an_android_17},
        {"android", logo_android},
        {"android small", logo_android_small},
        {"android robot", logo_an_robot},
        {"android_robot", logo_an_robot},
        {"android_bugdroid", logo_an_robot},
        {"bugdroid", logo_an_robot},
        {"lineageos", logo_lineage_os},
        {"lineage os", logo_lineage_os},
        {"lineage_os", logo_lineage_os},
    };
    return registry;
}

}
