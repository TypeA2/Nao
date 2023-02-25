// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include <span>
#include <cstddef>
#include <concepts>

#include <fmt/format.h>

class io_error : public std::runtime_error {
    public:
    template <typename... Args>
    io_error(fmt::format_string<Args...> fmt, Args&&... args)
        : std::runtime_error(fmt::format(fmt, std::forward<Args>(args)...)) {

    }
};

class file_stream {
    protected:
    size_t pos;
    
    public:
    file_stream();
    virtual ~file_stream() = default;

    /**
     * @brief Read at most `dest.size()` bytes into dest
     * 
     * @param dest Destination data buffer
     * @return size_t Total number of bytes read, may be less than the size of `dest`, or negative for an error
     */
    [[nodiscard]] virtual ssize_t read(std::span<std::byte> dest) = 0;

    /**
     * @brief Template overload for std::span
     * 
     * @tparam T Element type
     * @param dest Destination buffer
     * @return ssize_t Number of elements read
     */
    template <std::integral T>
    [[nodiscard]] ssize_t read(std::span<T> dest) {
        return read(std::as_writable_bytes(dest)) / sizeof(T);
    }

    /**
     * @brief Array overload for nicer code
     * 
     * @tparam T Array type
     * @tparam N Array size
     * @param dest Array to write data to
     * @return ssize_t Number of bytes read, or error
     */
    template <std::integral T, size_t N>
    [[nodiscard]] ssize_t read(std::array<T, N>& dest) {
        return read(std::span{ dest.begin(), dest.end() });
    }
};

#endif /* FILE_STREAM_H*/
