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

#include "naocore_defs.h"

namespace nao {
    template <typename CharT>
    class basic_string_view {
        public:
        using value_type = CharT;
        using size_type = size_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = pointer;
        using const_iterator = const_pointer;

        private:
        const_pointer _data = nullptr;
        size_type _size = 0;

        public:
        constexpr basic_string_view() noexcept = default;
        constexpr basic_string_view(const basic_string_view& other) noexcept = default;
        constexpr basic_string_view(const_pointer s, size_type count) : _data { s }, _size { count } { }
        constexpr basic_string_view(const_pointer s) : _data { s } {
            for (; *s; ++s) {
                ++_size;
            }
        }

        constexpr size_type size() const { return _size; }
        constexpr const_pointer data() const { return _data; }
    };

    NAOCORE_INSTANTIATE basic_string_view<char>;
    NAOCORE_INSTANTIATE basic_string_view<wchar_t>;

    using string_view = basic_string_view<char>;
    using wstring_view = basic_string_view<wchar_t>;
}
