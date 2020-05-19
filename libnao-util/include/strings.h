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

#include <string>
#include <chrono>

/**
 * Functions related to string conversions, etc.
 */

namespace strings {
    NAOUTIL_API std::wstring to_utf16(std::string_view utf8);
    NAOUTIL_API std::string to_utf8(std::wstring_view utf16);

    NAOUTIL_API std::string bytes(size_t n);
    NAOUTIL_API std::string bits(size_t n);

    NAOUTIL_API std::string percent(double v);

    NAOUTIL_API std::string time_hours(std::chrono::nanoseconds ns, bool ms = true);
    NAOUTIL_API std::string time_minutes(std::chrono::nanoseconds ns, bool ms = true);
}