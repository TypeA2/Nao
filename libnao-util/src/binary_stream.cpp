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
#include "binary_stream.h"

#include <fstream>
#include <format>

nao::binary_istream::binary_istream(std::unique_ptr<std::streambuf> rdbuf)
    // Pointer should remain the same here
    : std::istream{ rdbuf.get() }
    , _rdbuf{ std::move(rdbuf) }{
}


nao::binary_istream::binary_istream(const std::filesystem::path& path)
    : binary_istream{ std::unique_ptr<std::filebuf>{
        (new std::filebuf())->open(path,std::ios::in | std::ios::binary) } } {
    if (!_rdbuf)) {
        logger().error("Failed to open file \"{}\"", path);
        throw std::runtime_error(std::format("File \"{}\" does not exist", path));
    }
}


nao::binary_istream::binary_istream(binary_istream&& other) noexcept {

}
