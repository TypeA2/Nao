#pragma once

#include <spdlog/spdlog.h>

namespace nao {
    extern spdlog::logger log;

    spdlog::logger make_logger(std::string_view name);
    spdlog::logger make_logger(std::string_view name, spdlog::level::level_enum level);
}
