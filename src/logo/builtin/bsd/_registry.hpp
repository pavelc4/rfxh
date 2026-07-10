#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "logo_DragonFly.hpp"
#include "logo_Freebsd.hpp"
#include "logo_GhostBSD.hpp"
#include "logo_HardenedBSD.hpp"
#include "logo_MidnightBSD.hpp"
#include "logo_NetBSD.hpp"
#include "logo_OmniOS.hpp"
#include "logo_PCBSD.hpp"
#include "logo_SmartOS.hpp"
#include "logo_nomadbsd.hpp"
#include "logo_openbsd.hpp"
#include "logo_solaris.hpp"
#include "logo_solaris_small.hpp"

namespace rfxh::logo::builtin::cat_bsd {

using LogoFunc = std::function<std::vector<std::string>()>;

inline const std::unordered_map<std::string, LogoFunc>& get_registry() {
    static const std::unordered_map<std::string, LogoFunc> registry = {
        {"dragonfly", builtin::cat_bsd::logo_DragonFly},
        {"freebsd", builtin::cat_bsd::logo_Freebsd},
        {"ghostbsd", builtin::cat_bsd::logo_GhostBSD},
        {"hardenedbsd", builtin::cat_bsd::logo_HardenedBSD},
        {"midnightbsd", builtin::cat_bsd::logo_MidnightBSD},
        {"netbsd", builtin::cat_bsd::logo_NetBSD},
        {"omnios", builtin::cat_bsd::logo_OmniOS},
        {"pcbsd", builtin::cat_bsd::logo_PCBSD},
        {"smartos", builtin::cat_bsd::logo_SmartOS},
        {"nomadbsd", builtin::cat_bsd::logo_nomadbsd},
        {"openbsd", builtin::cat_bsd::logo_openbsd},
        {"solaris", builtin::cat_bsd::logo_solaris},
        {"solaris_small", builtin::cat_bsd::logo_solaris_small},
    };
    return registry;
}

}
