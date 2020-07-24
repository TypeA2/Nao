/*  This file is part of libnao-core.

    libnao-core is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao-core is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libnao-core.  If not, see <https://www.gnu.org/licenses/>.   */
#include "nao/internal.h"

#include <new>

#include <windows.h>

namespace nao {
    void* _nc_alloc_aligned(size_t bytes, size_t align) {
        return operator new(bytes, std::align_val_t { align });
    }

    void _nc_free_aligned(void* ptr, size_t align) {
        return operator delete(ptr, std::align_val_t { align });
    }

    size_t _nc_narrow_to_wide(const char* narrow, size_t len, wchar_t* wide) {
        size_t size = MultiByteToWideChar(
            CP_UTF8, MB_ERR_INVALID_CHARS,
            narrow, static_cast<int>(len),
            nullptr, 0);

        if (!wide) {
            return size;
        }

        wide[size] = 0;

        size_t converted = MultiByteToWideChar(
            CP_UTF8, MB_ERR_INVALID_CHARS,
            narrow, static_cast<int>(len),
            wide, static_cast<int>(size));

        return (converted == size) ? converted : 0;
    }

    size_t _nc_wide_to_narrow(const wchar_t* wide, size_t len, char* narrow) {
        size_t size = WideCharToMultiByte(
            CP_UTF8, WC_COMPOSITECHECK | WC_NO_BEST_FIT_CHARS,
            wide, static_cast<int>(len),
            nullptr, 0, nullptr, nullptr);

        if (!narrow) {
            return size;
        }

        narrow[size] = 0;

        size_t converted = WideCharToMultiByte(
            CP_UTF8, WC_COMPOSITECHECK | WC_NO_BEST_FIT_CHARS,
            wide, static_cast<int>(len),
            narrow, static_cast<int>(size), nullptr, nullptr);

        return (converted == size) ? converted : 0;
    }

}