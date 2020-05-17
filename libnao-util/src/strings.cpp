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
#include "strings.h"

#include "windows_min.h"

#include <stdexcept>

namespace strings {
    std::wstring to_utf16(const std::string& utf8) {
        int size = MultiByteToWideChar(
            CP_UTF8, MB_ERR_INVALID_CHARS,
            utf8.c_str(), -1,
            nullptr, 0);

        std::wstring conv(size, L'\0');
        int converted = MultiByteToWideChar(
            CP_UTF8, MB_ERR_INVALID_CHARS,
            utf8.c_str(), -1,
            conv.data(), size + 1);

        if (converted != size) {
            throw std::runtime_error(__FUNCTION__ ": conversion failed");
        }

        return conv;
    }

}