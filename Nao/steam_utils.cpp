#include "steam_utils.h"

#include "frameworks.h"
#include "utils.h"

#include "vdf_parser.h"

#include <filesystem>

namespace steam_utils {
    std::wstring steam_path() {
        DWORD type;
        DWORD size = MAX_PATH;
        WCHAR str[MAX_PATH] { };

        LSTATUS status = RegGetValueW(HKEY_CURRENT_USER,
            L"Software\\Valve\\Steam", L"SteamPath", RRF_RT_ANY, &type, str, &size);

        ASSERT(status == ERROR_SUCCESS);

        return str;
    }

    std::vector<std::wstring> steam_install_folders() {
        auto vdf_path = std::filesystem::path(
            steam_path() + L"/SteamApps/libraryfolders.vdf")
                .lexically_normal();

        std::ifstream in(vdf_path);

        tyti::vdf::object root = tyti::vdf::read(in);

        std::vector<std::wstring> res(root.attribs.size() - 1);

        res[0] = std::filesystem::absolute(steam_path()).lexically_normal();

        for (size_t i = 1; i <= (root.attribs.size() - 2); ++i) {
            res[i] = std::filesystem::absolute(
                root.attribs.at(std::to_string(i))).lexically_normal();
        }

        return res;
    }

    std::wstring game_path(const std::wstring& game, const std::wstring& subdir) {
        for (const std::wstring& dir : steam_install_folders()) {
            for (const auto& entry : std::filesystem::directory_iterator(dir + L"/SteamApps/common")) {
                if (is_directory(entry.path()) && entry.path().filename() == game) {
                    return std::filesystem::path(entry.path().wstring() + L'\\' + subdir).lexically_normal();
                }
            }
        }

        return std::wstring();
    }


}
