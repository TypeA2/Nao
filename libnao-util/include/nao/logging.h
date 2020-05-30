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

#include <sstream>

/**
 * Asynchronous logging, function interface
 */

namespace nao {
    template <typename T>
    const T& printable(const T& arg) { return arg; }

    NAOUTIL_API void cout(string str);

    // Print to the appropriate output stream
    template <typename... Args>
    void cout(Args&&... args) {
        std::stringstream ss;
        ((ss << printable(std::forward<Args>(args)) << ' '), ...);
        auto string = ss.str();
        static_cast<void(*)(nao::string)>(cout)(string.substr(0, string.size() - 1));
    }

    template <typename... Args>
    void coutln(Args&&... args) {
        cout(args..., '\n');
    }
}
