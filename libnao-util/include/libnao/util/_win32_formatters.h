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

#include <fmt/format.h>

#include <Windows.h>

template <>
struct fmt::formatter<GUID> {
    static constexpr auto parse(format_parse_context& ctx) {
        // Nothing to parse
        if (ctx.begin() != ctx.end()) {
            throw format_error("invalid GUID format");
        }

        return ctx.end();
    }

    template <typename FormatContext>
    constexpr auto format(const GUID& g, FormatContext& ctx) {
        return format_to(ctx.out(),
                         "{{{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}}}",
                         g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1],
                         join(g.Data4 + 2, g.Data4 + 8, ""));
    }
};