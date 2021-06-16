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

#include "logging.h"

#include <istream>
#include <filesystem>

namespace nao {
    class binary_istream : public std::istream {
        NAO_LOGGER(binary_istream);

        std::unique_ptr<std::streambuf> _rdbuf;

        public:
        /* Use the specified streambuf */
        explicit binary_istream(std::unique_ptr<std::streambuf> rdbuf);

        /* Open a file */
        explicit binary_istream(const std::filesystem::path& path);

        binary_istream(const binary_istream&) = delete;
        binary_istream& operator=(const binary_istream&) = delete;

        binary_istream(binary_istream&& other) noexcept;
        binary_istream& operator=(binary_istream&& other) noexcept;
    };
}
