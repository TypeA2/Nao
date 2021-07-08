/**
 *  This file is part of libnao-av.
 *
 *  libnao-av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-av.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <libnao/util/defs.h>

#include <vector>
#include <span>
#include <format>

namespace nao::av {
    enum class pixel_format {
        none,
        RGBA32,
    };

    /* Size in bytes for 1 pixel component in the given pixel format */
    [[nodiscard]] constexpr size_t pixel_format_size(pixel_format fmt) {
        using enum pixel_format;
        switch (fmt) {
            case none:  return 0;
            case RGBA32: return 4;
            default:     return -1;
        }
    }

    /* Simple text conversion */
    [[nodiscard]] constexpr std::string_view pixel_format_name(pixel_format fmt) {
        using enum pixel_format;
        switch (fmt) {
            case none:   return "(none)";
            case RGBA32: return "RGBA32";
            default:     return "(unknown)";
        }
    }

    /* Generic image class that can hold image data*/
    class image {
        std::vector<std::byte> _data;
        pixel_format _format = pixel_format::none;
        size _size{};

        public:
        image() = default;
        image(std::span<std::byte> data, pixel_format format, size size);

        image(const image& other) = default;
        image& operator=(const image& other) = default;

        image(image&& other) noexcept = default;
        image& operator=(image&& other) noexcept = default;
    };
}

namespace std {
    template <> struct formatter<nao::av::pixel_format> {
        
    };
}