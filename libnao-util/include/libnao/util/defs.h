/**
 *  This file is part of libnao-ui.
 *
 *  libnao-ui is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-ui is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-ui.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <limits>

namespace nao {
    struct position {
        long x, y;
    };

    struct size {
        long w, h;

        static constexpr size max() {
            return size{
                 .w = std::numeric_limits<long>::max(),
                 .h = std::numeric_limits<long>::max(),
            };
        }

        [[nodiscard]] size fit_in(size other) const;
    };

    struct margins {
        long top, right, bot, left;
    };
}
