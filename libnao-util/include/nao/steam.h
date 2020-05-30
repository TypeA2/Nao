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
#pragma once

#include "naoutil_defs.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <nao/error.h>
#include <nao/string_view.h>

namespace nao::steam {
    // Steam root path
    NAOUTIL_API expected<string> path();

    // A list of all steam install forlders
    //NAOUTIL_API std::vector<string> install_folders();

    // Retrieve a game's path, if possible
    NAOUTIL_API expected<string> game_path(string_view game);

    // Basic VDF parsing, only supports quoted identifiers
    namespace vdf {
        struct NAOUTIL_API object {
            class opaque;
            opaque* d;

            object();
            ~object();

            std::string& name();
            std::unordered_map<std::string, std::string>& attributes();
            std::unordered_map<std::string, object>& children();

            void print(std::ostream& os, size_t indent = 0) const;
        };

        NAOUTIL_API object parse(std::istream& in);
    }
}
