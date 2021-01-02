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

#include <spdlog/spdlog.h>

#define NAO_LOGGER(name) \
    static spdlog::logger& logger() { \
        static spdlog::logger logger = make_logger(#name, default_logging_level); \
        return logger; \
    }

namespace nao {
    extern spdlog::logger log;

    inline constexpr spdlog::level::level_enum default_logging_level =
#ifndef NDEBUG
        spdlog::level::debug;
#else
        spdlog::level::info;
#endif

    spdlog::logger make_logger(std::string_view name);
    spdlog::logger make_logger(std::string_view name, spdlog::level::level_enum level);
}
