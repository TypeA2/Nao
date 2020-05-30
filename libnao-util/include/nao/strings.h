/*  This file is part of libnao-util.

    libnao-util is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao-util is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libnao-util.  If not, see <https://www.gnu.org/licenses/>.   */
#pragma once

#include "naoutil_defs.h"

#include <nao/string.h>

/**
 * Functions related to string conversions, etc.
 */

namespace nao {
    NAOUTIL_API string bytes(size_t n);
    NAOUTIL_API string bits(size_t n);

    NAOUTIL_API string percent(double v);

    NAOUTIL_API string time_hours(uint64_t nanoseconds, bool ms = true);
    NAOUTIL_API string time_minutes(uint64_t nanoseconds, bool ms = true);
}