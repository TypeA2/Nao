// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>

#include <fmt/format.h>

class archive_error : public std::runtime_error {
    public:
    template <typename... Args>
    archive_error(fmt::format_string<Args...> fmt, Args&&... args)
        : std::runtime_error(fmt::format(fmt, std::forward<Args>(args)...)) {

    }
};

class io_error : public std::runtime_error {
    public:
    template <typename... Args>
    io_error(fmt::format_string<Args...> fmt, Args&&... args)
        : std::runtime_error(fmt::format(fmt, std::forward<Args>(args)...)) {

    }
};

class decode_error : public std::runtime_error {
    public:
    template <typename... Args>
    decode_error(fmt::format_string<Args...> fmt, Args&&... args)
        : std::runtime_error(fmt::format(fmt, std::forward<Args>(args)...)) {

    }
};

#endif /* EXCEPTIONS_HPP */
