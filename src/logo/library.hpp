#pragma once
#include <string>
#include <vector>

namespace rfxh::logo {

const std::vector<std::string>* get_builtin_logo(const std::string& name);
std::vector<std::string> get_builtin_logo_names();

}
