#include "steam_utils.h"

#include "frameworks.h"
#include "utils.h"

#include "vdf_parser.h"

#include <filesystem>

#include <strings.h>

namespace steam_utils {
    std::string steam_path() {
        DWORD type;
        DWORD size = MAX_PATH;
        WCHAR str[MAX_PATH] { };

        LSTATUS status = RegGetValueW(HKEY_CURRENT_USER,
            L"Software\\Valve\\Steam", L"SteamPath", RRF_RT_ANY, &type, str, &size);

        ASSERT(status == ERROR_SUCCESS);

        return strings::to_utf8(str);
    }

    std::vector<std::string> steam_install_folders() {
        auto vdf_path = std::filesystem::path(
            steam_path() + "\\SteamApps\\libraryfolders.vdf")
                .lexically_normal();

        std::ifstream in(vdf_path);

        tyti::vdf::object root = tyti::vdf::read(in);

        std::vector<std::string> res(root.attribs.size() - 1);

        res[0] = std::filesystem::absolute(steam_path()).lexically_normal().string();

        for (size_t i = 1; i <= (root.attribs.size() - 2); ++i) {
            res[i] = std::filesystem::absolute(
                root.attribs.at(std::to_string(i))).lexically_normal().string();
        }

        return res;
    }

    std::string game_path(const std::string& game, const std::string& subdir) {
        for (const std::string& dir : steam_install_folders()) {
            try {
                for (const auto& entry : std::filesystem::directory_iterator(dir + "\\SteamApps\\common")) {
                    if (is_directory(entry.path()) && entry.path().filename() == game) {
                        return std::filesystem::path(entry.path().string() + '\\' + subdir).lexically_normal().string();
                    }
                }
            } catch (const std::filesystem::filesystem_error&) {
                // Doesn't exist
            }
        }

        return std::string();
    }


}
