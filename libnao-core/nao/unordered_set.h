/*  This file is part of libnao-core.

    libnao-core is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao-core is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libnao-core.  If not, see <https://www.gnu.org/licenses/>.   */
#pragma once

#include <functional>
#include <unordered_set>

#include "naocore_defs.h"


namespace nao {
    template <
        typename Key,
        typename Hash = std::hash<Key>,
        typename KeyEqual = std::equal_to<Key>>
    class unordered_set {
        public:
        using key_type = Key;
        using value_type = Key;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using hasher = Hash;
        using key_equal = KeyEqual;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = Key*;
        using const_pointer = const Key*;
    };
}