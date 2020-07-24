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
#pragma once

#include "naocore_defs.h"

//
// Internally used functions
//

namespace nao {
    NAOCORE_IMPL void* NAOCORE_DECL _nc_alloc_aligned(size_t bytes, size_t align);
    NAOCORE_IMPL void NAOCORE_DECL _nc_free_aligned(void* ptr, size_t align);

    template <typename T>
    T* _nc_alloc_aligned(size_t count) {
        return static_cast<T*>(_nc_alloc_aligned(count * sizeof(T), alignof(T)));
    }

    template <typename T>
    void _nc_free_aligned(T* ptr) {
        _nc_free_aligned(ptr, alignof(T));
    }

    //
    // Known-length narrow (UTF-8) string to wide (UTF-16). Nullptr on error.
    // if wide == nullptr, return needed size (excluding null terminator)
    //
    NAOCORE_IMPL size_t NAOCORE_DECL _nc_narrow_to_wide(const char* narrow, size_t len, wchar_t* wide);

    //
    // Same but for UTF-16 -> UTF-8
    //
    NAOCORE_IMPL size_t NAOCORE_DECL _nc_wide_to_narrow(const wchar_t* wide, size_t len, char* narrow);
}