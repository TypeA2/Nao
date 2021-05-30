/**
 *  This file is part of naofs.
 *
 *  naofs is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  naofs is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with naofs.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <libnao/util/logging.h>
#include <libnao/util/win32_compat.h>

#include <filesystem>
#include <unordered_map>

class projection {
    NAO_LOGGER(projection);

    public:
    using path = std::filesystem::path;

    static constexpr std::string_view instance_file = ".naofs";

    private:
    path _src;
    path _root;

    GUID _instance;

    std::unordered_map<GUID, bool> _active_enums;

    public:
    projection(const path& source, const path& root);
    projection(const projection&) = delete;
    projection(projection&&) = delete;
    projection& operator=(const projection&) = delete;
    projection& operator=(projection&&) = delete;

    private:
    void _prepare_root();
    void _start();
};
