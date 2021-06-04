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
#include "concepts.h"

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

    /**
     * HGDIOBJ wrapper
     */
    class gdi_object {
        HGDIOBJ _obj{};

        public:
        gdi_object() = default;
        explicit(false) gdi_object(HGDIOBJ obj);
        ~gdi_object();

        gdi_object(gdi_object&& other) noexcept;
        gdi_object& operator=(gdi_object&& other) noexcept;

        gdi_object(const gdi_object&) = delete;
        gdi_object& operator=(const gdi_object&) = delete;

        [[nodiscard]] HGDIOBJ handle() const;

        template<typename T> requires explicitly_convertible_to<HGDIOBJ, T>
        [[nodiscard]] T handle() const {
            return static_cast<T>(handle());
        }
    };

    /* Set or unset a specific style for the given window */
    void set_style(HWND hwnd, DWORD style, bool enable);
}
