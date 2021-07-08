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

#include "defs.h"

#include <fmt/format.h>

#include <Windows.h>

#include <filesystem>
#include <format>

namespace nao::detail {
    template <typename Ctx, typename Err>
    struct no_parse_base {
        static constexpr auto parse(Ctx& ctx) {
            // Nothing to parse
            if (ctx.begin() != ctx.end()) {
                throw Err("invalid format");
            }

            return ctx.end();
        }
    };

    using no_parse_fmt = no_parse_base<fmt::format_parse_context, fmt::format_error>;
    using no_parse_std = no_parse_base<std::format_parse_context, std::format_error>;

    /* Formatter implementations */
    struct format_guid {
        template <typename FormatContext>
        constexpr auto format(const GUID& g, FormatContext& ctx) {
            return format_to(ctx.out(),
                "{{{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}}}",
                g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1],
                fmt::join(g.Data4 + 2, g.Data4 + 8, ""));
        }
    };

    struct format_std_fs_path {
        template <typename FormatContext>
        constexpr auto format(const std::filesystem::path& p, FormatContext& ctx) {
            return fmt::format_to(ctx.out(), "{}", p.string());
        }
    };

    struct format_position {
        template <typename FormatContext>
        constexpr auto format(const position& pos, FormatContext& ctx) {
            return fmt::format_to(ctx.out(), "({}, {})", pos.x, pos.y);
        }
    };

    struct format_size {
        template <typename FormatContext>
        constexpr auto format(const size& s, FormatContext& ctx) {
            return fmt::format_to(ctx.out(), "{}x{}", s.w, s.h);
        }
    };

    struct format_margins {
        template <typename FormatContext>
        constexpr auto format(const margins& m, FormatContext& ctx) {
            return fmt::format_to(ctx.out(), "{{{}, {}, {}, {}}}", m.top, m.right, m.bot, m.left);
        }
    };
}

template <> struct fmt::formatter<GUID> : nao::detail::no_parse_fmt, nao::detail::format_guid {};
template <> struct std::formatter<GUID> : nao::detail::no_parse_std, nao::detail::format_guid {};

template <> struct fmt::formatter<std::filesystem::path>
    : nao::detail::no_parse_fmt, nao::detail::format_std_fs_path {};
template <> struct std::formatter<std::filesystem::path>
    : nao::detail::no_parse_std, nao::detail::format_std_fs_path {};

template <> struct fmt::formatter<nao::position>
    : nao::detail::no_parse_fmt, nao::detail::format_position {};
template <> struct std::formatter<nao::position>
    : nao::detail::no_parse_std, nao::detail::format_position {};

template <> struct fmt::formatter<nao::size>
    : nao::detail::no_parse_fmt, nao::detail::format_size {};
template <> struct std::formatter<nao::size>
    : nao::detail::no_parse_std, nao::detail::format_size {};

template <> struct fmt::formatter<nao::margins>
    : nao::detail::no_parse_fmt, nao::detail::format_margins {};
template <> struct std::formatter<nao::margins>
    : nao::detail::no_parse_std, nao::detail::format_margins {};

