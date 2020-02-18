#pragma once

#include <istream>
#include <mutex>
#include <filesystem>

#include "concepts.h"

class binary_istream {
    public:
    using pos_type = std::istream::pos_type;

    using seekdir = std::istream::seekdir;

    explicit binary_istream(const std::string& path);
    explicit binary_istream(const std::filesystem::path& path);

    explicit binary_istream(binary_istream&& other) noexcept;
    explicit binary_istream(const binary_istream& other) = delete;

    virtual ~binary_istream() = default;

    virtual pos_type tellg() const;
    virtual binary_istream& seekg(pos_type pos);
    virtual binary_istream& seekg(pos_type pos, seekdir dir);
    virtual bool eof() const;
    virtual binary_istream& ignore(std::streamsize max,
        std::istream::int_type delim = std::istream::traits_type::eof());

    virtual bool good() const;

    virtual std::streamsize gcount() const;

    virtual void clear();

    // Seek with std::ios::cur
    binary_istream& rseek(pos_type pos);

    virtual binary_istream& read(char* buf, std::streamsize count);

    // Read count elements, each size bytes, into buf
    template <concepts::pointer T>
    binary_istream& read(T buf, std::streamsize size, std::streamsize count = 1) {
        return read(reinterpret_cast<char*>(buf), count* size);
    }
    
    // Read a POD value array
    template <concepts::arithmetic T>
    binary_istream& read(T* buf, std::streamsize count = 1) {
        return read(reinterpret_cast<char*>(buf), count * sizeof(T));
    }

    // Read a POD value array
    template <concepts::arithmetic T, size_t size>
    binary_istream& read(T(&buf)[size]) {
        return read(buf, size * sizeof(T));
    }

    // Read any container which exposes ::data(), ::size() and ::value_type
    template <concepts::readable_container Container>
    binary_istream& read(Container& buf) {
        return read(buf.data(), buf.size() * sizeof(Container::value_type));
    }

    // Read an arithmetic value in the specified endianness
    template <concepts::arithmetic T, std::endian endian = std::endian::native>
    T read() {
        T temp {};
        char buf[sizeof(T)];
        read(buf, sizeof(T));

        if constexpr (endian == std::endian::little) {
            for (size_t i = 0; i < sizeof(T); ++i) {
                temp |= (T(uint8_t(buf[i])) << (i * CHAR_BIT));
            }
        } else {
            for (size_t i = (sizeof(T) - 1); i >= 0; --i) {
                temp |= (T(uint8_t(buf[i])) << (i * CHAR_BIT));
            }
        }

        return temp;
    }

    // Read an arithmetic value into the argument
    template <concepts::arithmetic T, std::endian endian = std::endian::native>
    binary_istream& read(T& val) {
        val = read<T>();

        return *this;
    }

    protected:
    std::unique_ptr<std::istream> file;
    mutable std::mutex mutex;
};

using istream_ptr = std::shared_ptr<binary_istream>;
