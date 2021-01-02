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

#include "libnao_util.h"

#include "logging.h"

#include <libnao/encoding.h>

#include <Windows.h>

#include <vector>
#include <string>
#include <algorithm>
#include <span>

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR lpCmdLine, _In_ int) {
    int argc;
    LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);

    // Construct all UTF-8 strings
    std::vector<std::string> argv_vec(argc);
    std::ranges::transform(std::span{ argv, static_cast<size_t>(argc) },
        argv_vec.begin(), nao::wide_to_utf8);

    LocalFree(argv);

    std::vector<char*> argv_ptrs(argc);
    std::ranges::transform(argv_vec, argv_ptrs.begin(),
        [](std::string& v) { return v.data(); });

    nao::log.debug("Calling libnao_main (at {}) as:", reinterpret_cast<void*>(libnao_main));
    nao::log.debug(format("    {}", fmt::join(argv_vec, " ")));

    return libnao_main(argc, argv_ptrs.data());
}

namespace nao::libnao_util {
    bool init() {
        return true;
    }
}
