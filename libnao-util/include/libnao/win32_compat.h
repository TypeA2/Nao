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
#pragma once

#include "_win32_formatters.h"

static_assert(sizeof(void*) == 8, "Must be on a 64-bit platform");
static_assert(sizeof(GUID) == 16, "GUID type must have no padding");

namespace std {
    template <>
    struct hash<GUID> {
        size_t operator()(const GUID& g) const noexcept {
            const auto* data = reinterpret_cast<const uint64_t*>(&g);

            return data[0] ^ data[1];
        }
    };
}

namespace nao::win32 {
    struct guid_compare;
}
