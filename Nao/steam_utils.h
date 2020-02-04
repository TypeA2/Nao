#pragma once

#include <string>
#include <vector>

namespace steam_utils {
    std::string steam_path();

    std::vector<std::string> steam_install_folders();

    std::string game_path(const std::string& game, const std::string& subdir = std::string());
}
