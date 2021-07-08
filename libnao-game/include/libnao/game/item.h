/**
 *  This file is part of libnao-game.
 *
 *  libnao-game is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-game is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-game.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <string>

namespace nao::game {
    class item_handler;

    /* A generic structure that represents a file or folder */
    class item {
        /* The item handler that this item belongs to */
        item_handler& _owner;

        std::string _name;
        std::string _desc;

        uint64_t _size;
    };
}