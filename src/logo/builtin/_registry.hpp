#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "lnx/_registry.hpp"
#include "mac/_registry.hpp"
#include "win/_registry.hpp"
#include "bsd/_registry.hpp"
#include "and/_registry.hpp"

namespace rfxh::logo {

inline const std::vector<std::string>* get_builtin_logo(const std::string& name) {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    static thread_local std::vector<std::string> result;
    
    auto& linux_reg = builtin_cat_linux::get_registry();
    auto it = linux_reg.find(lower);
    if (it != linux_reg.end()) { result = it->second(); return &result; }
    
    auto& macos_reg = builtin_cat_macos::get_registry();
    it = macos_reg.find(lower);
    if (it != macos_reg.end()) { result = it->second(); return &result; }
    
    auto& win_reg = builtin_cat_windows::get_registry();
    it = win_reg.find(lower);
    if (it != win_reg.end()) { result = it->second(); return &result; }
    
    auto& bsd_reg = builtin_cat_bsd::get_registry();
    it = bsd_reg.find(lower);
    if (it != bsd_reg.end()) { result = it->second(); return &result; }
    
    auto& android_reg = builtin_cat_android::get_registry();
    it = android_reg.find(lower);
    if (it != android_reg.end()) { result = it->second(); return &result; }
    
    return nullptr;
}

inline std::vector<std::string> get_builtin_logo_names() {
    std::vector<std::string> names;
    for (const auto& reg : {&builtin_cat_linux::get_registry(), &builtin_cat_macos::get_registry(),
                            &builtin_cat_windows::get_registry(), &builtin_cat_bsd::get_registry(),
                            &builtin_cat_android::get_registry()}) {
        names.reserve(names.size() + reg->size());
        for (const auto& [name, _] : *reg) {
            names.push_back(name);
        }
    }
    return names;
}

}
