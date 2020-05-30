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

#ifdef LIBNAOCORE_EXPORTS
#define NAOCORE_API __declspec(dllexport)
#else
#define NAOCORE_API __declspec(dllimport)
#endif

#define NAOCORE_DECL __cdecl
#define NAOCORE_IMPL extern "C" NAOCORE_API
#define NAOCORE_INSTANTIATE extern template class
