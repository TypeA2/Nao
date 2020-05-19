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

namespace steam {
    // Steam root path
    NAOUTIL_API std::string path();

    // A list of all steam install forlders
    NAOUTIL_API std::vector<std::string> install_folders();

    // Retrieve a game's path, if possible
    NAOUTIL_API std::string game_path(std::string_view game);
    NAOUTIL_API std::string game_path(std::string_view game, bool& found);
}
