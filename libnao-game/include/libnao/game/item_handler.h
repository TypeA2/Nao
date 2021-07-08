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

#include <libnao/util/binary_stream.h>

#include <string>
#include <type_traits>

#include <cstdint>

namespace nao::game {
    enum class item_handler_tag : uint64_t {
        none    = 0b00,

        /* The file handler represents that of a single file */
        file    = 0b01,

        /* The file handler contains sub-items. */
        subitem = 0b10,
    };

    [[nodiscard]] constexpr item_handler_tag operator|(
        item_handler_tag left, item_handler_tag right) {

        using enum_type = std::underlying_type_t<item_handler_tag>;

        return static_cast<item_handler_tag>(
            static_cast<enum_type>(left) | static_cast<enum_type>(right));
    }

    [[nodiscard]] constexpr item_handler_tag operator&(
        item_handler_tag left, item_handler_tag right) {

        using enum_type = std::underlying_type_t<item_handler_tag>;

        return static_cast<item_handler_tag>(
            static_cast<enum_type>(left) & static_cast<enum_type>(right));
    }

    class file_handler;
    class subitem_handler;

    /* Map enum values to specific implementations */
    template <item_handler_tag> struct file_handler_type { };

    template <> struct file_handler_type<item_handler_tag::file> {
        using type = file_handler;
    };

    template <> struct file_handler_type<item_handler_tag::subitem> {
        using type = subitem_handler;
    };

    template <item_handler_tag Tag>
    using file_handler_type_t = typename file_handler_type<Tag>::type;
    
    /**
     * Base class that acts as a catch-all for handling files and folders. The tag system allows for
     * safe downcasting to specific kinds of handlers, and this base class only contains this and
     * a file path.
     */
    class item_handler {
        std::string _path;

        public:
        explicit item_handler(std::string_view path);

        virtual ~item_handler() = default;

        [[nodiscard]] virtual item_handler_tag tag() const = 0;

        [[nodiscard]] std::string_view path() const;
    };

    /* Simple base handler for files with a corresponding stream */
    class file_handler : public virtual item_handler {
        binary_istream _stream;

        public:
        file_handler(std::string_view path, binary_istream stream);

        [[nodiscard]] item_handler_tag tag() const override;
    };
}
