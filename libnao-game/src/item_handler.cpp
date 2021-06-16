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
#include "item_handler.h"

nao::game::item_handler::item_handler(std::string_view path)
    : _path{ path } {
    
}

std::string_view nao::game::item_handler::path() const {
    return _path;
}


nao::game::file_handler::file_handler(std::string_view path, binary_istream stream)
    : item_handler{ path }
    , _stream{ std::move(stream) }{
    
}
