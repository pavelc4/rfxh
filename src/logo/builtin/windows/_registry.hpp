#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "logo_Pengwin.hpp"
#include "logo_Windows.hpp"
#include "logo_Windows_11.hpp"
#include "logo_Windows_11_small.hpp"
#include "logo_Windows_8.hpp"
#include "logo_Windows_95.hpp"
#include "logo_Windows_Server_2025.hpp"

namespace rfxh::logo::builtin::cat_windows {

using LogoFunc = std::function<std::vector<std::string>()>;

inline const std::unordered_map<std::string, LogoFunc>& get_registry() {
    static const std::unordered_map<std::string, LogoFunc> registry = {
        {"pengwin", builtin::cat_windows::logo_Pengwin},
        {"windows", builtin::cat_windows::logo_Windows},
        {"windows 11", builtin::cat_windows::logo_Windows_11},
        {"windows 11_small", builtin::cat_windows::logo_Windows_11_small},
        {"windows 8", builtin::cat_windows::logo_Windows_8},
        {"windows 95", builtin::cat_windows::logo_Windows_95},
        {"windows server 2025", builtin::cat_windows::logo_Windows_Server_2025},
    };
    return registry;
}

}
