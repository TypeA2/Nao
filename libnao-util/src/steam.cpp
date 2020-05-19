/*  This file is part of libnao-util.

    libnao-util is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao-util is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libnao-util.  If not, see <https://www.gnu.org/licenses/>.   */
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "steam.h"

#include "windows_min.h"
#include "strings.h"

#include <stdexcept>
#include <filesystem>
#include <fstream>

// TODO in-house VDF parser
#include "vdf_parser.h"

namespace steam {
    std::string path() {
        DWORD type;
        DWORD size = MAX_PATH;
        WCHAR str[MAX_PATH] { };

        LSTATUS status = RegGetValueW(
            HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath",
            RRF_RT_REG_SZ, &type, str, &size);

        if (status != ERROR_SUCCESS) {
            throw std::runtime_error(__FUNCTION__ ": Steam path registry retrieval failed");
        }

        return strings::to_utf8(str);
    }

    std::vector<std::string> install_folders() {
        auto vdf_path = std::filesystem::path {
            path() + "\\SteamApps\\libraryfolders.vdf" }.lexically_normal();

        std::ifstream in { vdf_path };

        tyti::vdf::object root = tyti::vdf::read(in);

        std::vector<std::string> folders { root.attribs.size() - 1 };

        folders[0] = path();

        for (size_t i = 1; i < folders.size(); ++i) {
            folders[i] = root.attribs.at(std::to_string(i));
        }

        std::transform(folders.begin(), folders.end(), folders.begin(), [](auto& path) {
            return std::filesystem::absolute(path).lexically_normal().string();
            });

        return folders;
    }

    std::string game_path(std::string_view game) {
        bool found;
        std::string path = game_path(game, found);

        if (!found) {
            throw std::runtime_error(__FUNCTION__ ": game path not found");
        }

        return path;
    }

    std::string game_path(std::string_view game, bool& found) {
        for (const std::string& dir : install_folders()) {
            try {
                std::filesystem::directory_iterator it { dir + "\\SteamApps\\common" };
                for (const auto& entry : it) {
                    if (is_directory(entry) && entry.path().filename() == game) {
                        found = true;
                        return absolute(entry).lexically_normal().string();
                    }
                }
            }  catch (const std::filesystem::filesystem_error&) {
                // Directory does not exist
            }
        }

        found = false;
        return {};
    }


}
