// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FILE_STREAM_HPP
#define FILE_STREAM_HPP

#include <span>
#include <cstddef>
#include <concepts>
#include <bit>
#include <vector>

#include "util/exceptions.hpp"

class file_stream {
    protected:
    std::streamoff pos;
    
    public:
    file_stream();
    virtual ~file_stream() = default;

    /**
     * @brief Retrieve the total size for the underlying file
     * 
     * @return std::streamsize 
     */
    [[nodiscard]] virtual std::streamsize size() const = 0;

    /**
     * @brief Retrieve the current position in the stream
     * 
     * @return std::streampos 
     */
    [[nodiscard]] virtual std::streamoff tell() const;

    /**
     * @brief Seek to a given absolute byte offset
     * 
     * @param pos 
     */
    virtual void seek(std::streamoff new_pos) = 0;

    /**
     * @brief Read at most `dest.size()` bytes into dest
     * 
     * @param dest Destination data buffer
     * @return std::streamsize Total number of bytes read, may be less than the size of `dest`, or negative for an error
     */
    [[nodiscard]] virtual std::streamsize read(std::span<std::byte> dest) = 0;

    /**
     * @brief Read a single byte and return it
     * 
     * @return std::byte
     */
    [[nodiscard]] virtual std::byte getb() = 0;

    /**
     * @brief Read a single character and return it
     * 
     * @return char 
     */
    [[nodiscard]] virtual char getc();

    /**
     * @brief Template overload for std::span
     * 
     * @tparam T Element type
     * @param dest Destination buffer
     * @return int64_t Number of elements read
     */
    template <std::integral T>
    [[nodiscard]] std::streamsize read(std::span<T> dest) {
        return read(std::as_writable_bytes(dest)) / sizeof(T);
    }

    /**
     * @brief Array overload for nicer code
     * 
     * @tparam T Array type
     * @tparam N Array size
     * @param dest Array to write data to
     * @return int64_t Number of bytes read, or error
     */
    template <std::integral T, size_t N>
    [[nodiscard]] std::streamsize read(std::array<T, N>& dest) {
        return read(std::span{ dest.begin(), dest.end() });
    }

    template <typename T> requires std::integral<T> || std::same_as<T, std::byte>
    [[nodiscard]] std::streamsize read(std::vector<T>& dest) {
        return read(std::span{ dest.begin(), dest.end() });
    }

    template <typename T, std::endian E> requires std::integral<T> || std::is_floating_point_v<T>
    [[nodiscard]] T read_pod() {
        std::array<std::byte, sizeof(T)> res;
        if (std::streamsize bytes_read = read(res); bytes_read != sizeof(T)) {
            throw io_error("expected to read {} bytes, instead got {} bytes", sizeof(T), bytes_read);
        }

        if constexpr (sizeof(T) == 1 || std::endian::native == E) {
            return std::bit_cast<T>(res);
        } else {
            if constexpr (std::integral<T>) {
                return std::byteswap(std::bit_cast<T>(res));
            } else {
                /* Floats have only 2 supported sizes */
                if constexpr (std::same_as<T, float>) {
                    static_assert(sizeof(float) == sizeof(uint32_t), "weird bytes");
                    return std::bit_cast<float>(std::byteswap(std::bit_cast<uint32_t>(res)));
                } else if constexpr (std::same_as<T, double>) {
                    static_assert(sizeof(double) == sizeof(uint64_t), "more weird bytes");
                    return std::bit_cast<double>(std::byteswap(std::bit_cast<uint64_t>(res)));
                } else {
                    static_assert(std::integral<T>, "unsupported floating point type");
                }
            }
        }
    }

    /**
     * @brief Read an 8-bit unsigned integer (endian-agnostic)
     * 
     * @return uint8_t 
     */
    [[nodiscard]] virtual uint8_t read_u8();

    /**
     * @brief Read a little-endian 16-bit unsigned integer
     * 
     * @return uint16_t 
     */
    [[nodiscard]] virtual uint16_t read_u16le();

    /**
     * @brief Read a little-endian 32-bit unsigned integer
     * 
     * @return uint32_t 
     */
    [[nodiscard]] virtual uint32_t read_u32le();

    /**
     * @brief Read a little-endian 64-bit unsigned integer
     * 
     * @return uint64_t 
     */
    [[nodiscard]] virtual uint64_t read_u64le();

    /**
     * @brief Read a big-endian 16-bit unsigned integer
     * 
     * @return uint16_t 
     */
    [[nodiscard]] virtual uint16_t read_u16be();

    /**
     * @brief Read a big-endian 32-bit unsigned integer
     * 
     * @return uint32_t 
     */
    [[nodiscard]] virtual uint32_t read_u32be();

    /**
     * @brief Read a big-endian 64-bit unsigned integer
     * 
     * @return uint64_t 
     */
    [[nodiscard]] virtual uint64_t read_u64be();

    /**
     * @brief Read an 8-bit signed integer (endian-agnostic)
     * 
     * @return int8_t 
     */
    [[nodiscard]] virtual uint8_t read_s8();

    /**
     * @brief Read a little-endian 16-bit signed integer
     * 
     * @return int16_t 
     */
    [[nodiscard]] virtual uint16_t read_s16le();

    /**
     * @brief Read a little-endian 32-bit signed integer
     * 
     * @return int32_t 
     */
    [[nodiscard]] virtual uint32_t read_s32le();

    /**
     * @brief Read a little-endian 64-bit signed integer
     * 
     * @return int64_t 
     */
    [[nodiscard]] virtual uint64_t read_s64le();

    /**
     * @brief Read a big-endian 16-bit signed integer
     * 
     * @return int16_t 
     */
    [[nodiscard]] virtual uint16_t read_s16be();

    /**
     * @brief Read a big-endian 32-bit signed integer
     * 
     * @return int32_t 
     */
    [[nodiscard]] virtual uint32_t read_s32be();

    /**
     * @brief Read a big-endian 64-bit signed integer
     * 
     * @return int64_t 
     */
    [[nodiscard]] virtual uint64_t read_s64be();

    /**
     * @brief Read a little-endian 32-bit float
     * 
     * @return float 
     */
    [[nodiscard]] virtual float read_f32le();

    /**
     * @brief Read a big-endian 32-bit float
     * 
     * @return float 
     */
    [[nodiscard]] virtual float read_f32be();

    /**
     * @brief Read a little-endian 64-bit float
     * 
     * @return double 
     */
    [[nodiscard]] virtual double read_f64le();

    /**
     * @brief Read a big-endian 64-bit float
     * 
     * @return double 
     */
    [[nodiscard]] virtual double read_f64be();

    /**
     * @brief Read a null- (or `term`-)terminated string.
     * 
     * @return std::string 
     */
    [[nodiscard]] virtual std::string read_cstring(char term = '\0'); 
};

#endif /* FILE_STREAM_HPP */
