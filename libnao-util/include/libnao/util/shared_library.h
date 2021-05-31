/**
 *  This file is part of libnao-util.
 *
 *  libnao-util is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-util is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-util.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <string_view>

#include <Windows.h>

namespace nao {
    class shared_library {
        HMODULE _handle{};

        public:
        explicit shared_library(std::string_view name);
        ~shared_library();

        shared_library(shared_library&& other) noexcept;
        shared_library& operator=(shared_library&& other) noexcept;

        shared_library(const shared_library&) = delete;
        shared_library& operator=(const shared_library&) = delete;

        [[nodiscard]] HMODULE handle() const;
    };
}