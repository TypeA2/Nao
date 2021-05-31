/**
 *  This file is part of libnao-util.
 *
 *  libnao-util is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-util is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-util.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "shared_library.h"

#include <cassert>

#include "encoding.h"

nao::shared_library::shared_library(std::string_view name)
    : _handle{ LoadLibraryW(utf8_to_wide(name).c_str()) } {
    assert(_handle);
}

nao::shared_library::~shared_library() {
    if (_handle) {
        FreeLibrary(_handle);
    }
}

nao::shared_library::shared_library(shared_library&& other) noexcept {
    _handle = std::exchange(other._handle, nullptr);
}

nao::shared_library& nao::shared_library::operator=(shared_library&& other) noexcept {
    _handle = std::exchange(other._handle, nullptr);

    return *this;
}

HMODULE nao::shared_library::handle() const {
    return _handle;
}
