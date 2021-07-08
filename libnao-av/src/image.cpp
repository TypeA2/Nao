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
#include "image.h"

#include <libnao/util/formatters.h>

#include <stdexcept>

nao::av::image::image(std::span<std::byte> data, pixel_format format, size size)
    : _data{ data.begin(), data.end() }
    , _format{ format }
    , _size{ size } {
    if (size_t expected_bytes = (_size.w * _size.h * pixel_format_size(format));
        expected_bytes != data.size_bytes()) {
        throw std::invalid_argument(
            fmt::format("Expected {} bytes for {} image of size {}, but got {}",
                expected_bytes, pixel_format_name(format), size, data.size_bytes()));
    }
}
