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
