#pragma once

#include <string>
#include <vector>

namespace steam_utils {
    std::wstring steam_path();

    std::vector<std::wstring> steam_install_folders();

    std::wstring game_path(const std::wstring& game,
        const std::wstring& subdir = std::wstring());
}
