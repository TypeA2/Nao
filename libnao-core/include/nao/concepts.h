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

#include <concepts>

namespace nao {
    template <typename A, typename B>
    concept addable = requires (A a, B b) {
        { a + b };
    };

    template <typename T, typename CharT>
    concept string_like = requires (T s) {
        { s.c_str() } -> std::convertible_to<const CharT*>;
        { s.size()  } -> std::convertible_to<size_t>;
    };

    template <typename A, typename B>
    concept safe_forwarding_reference = !std::convertible_to<std::decay_t<A>*, B*>;
}
