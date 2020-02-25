#pragma once

#include <istream>
#include <mutex>
#include <filesystem>

#include "concepts.h"
#include "utils.h"

template <size_t> struct fixed_uint { };
template <>       struct fixed_uint<8> { using type = uint8_t; };
template <>       struct fixed_uint<16> { using type = uint16_t; };
template <>       struct fixed_uint<32> { using type = uint32_t; };
template <>       struct fixed_uint<64> { using type = uint64_t; };

template <size_t size>
using fixed_uint_t = typename fixed_uint<size>::type;

namespace detail {
    template <bool min8, bool min16, bool min32>
    struct fixed_size_integral { };

    template <>
    struct fixed_size_integral<false, false, false> {
        using type = fixed_uint_t<8>;
    };

    template <>
    struct fixed_size_integral<true, false, false> {
        using type = fixed_uint_t<16>;
    };

    template <>
    struct fixed_size_integral<true, true, false> {
        using type = fixed_uint_t<32>;
    };

    template <>
    struct fixed_size_integral<true, true, true> {
        using type = fixed_uint_t<64>;
    };
}

template <size_t min>
using min_uint_t = typename detail::fixed_size_integral<(min > 8), (min > 16), (min > 32)>::type;


class binary_istream {
    public:
    using pos_type = std::istream::pos_type;
    using seekdir = std::istream::seekdir;

    explicit binary_istream(const std::string& path);
    explicit binary_istream(const std::filesystem::path& path);

    // file pointer constructor
    explicit binary_istream(const std::shared_ptr<std::istream>& file);

    explicit binary_istream(binary_istream&& other) noexcept;
    explicit binary_istream(const binary_istream& other) = delete;

    // Construct from a Win32 resource
    explicit binary_istream(int resource, const std::string& type = "binary");

    // Construct from an existing streambuf
    explicit binary_istream(std::unique_ptr<std::streambuf> buf);

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
    binary_istream& read(T buf, std::streamsize size, std::streamsize count) {
        return read(reinterpret_cast<char*>(buf), count* size);
    }
    
    // Read a POD value array
    template <concepts::pod T>
    binary_istream& read(T* buf, std::streamsize bytes) {
        return read(reinterpret_cast<char*>(buf), bytes);
    }

    // Read a POD value array
    template <concepts::pod T, size_t size>
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
        char buf[sizeof(T)] {};
        read(buf, sizeof(T));

        if constexpr (endian != std::endian::native) {
            // Byte swap
            std::reverse(std::begin(buf), std::end(buf));
        }
        memcpy(&temp, buf, sizeof(T));

        return temp;
    }

    // Read an arithmetic value into the argument
    template <concepts::arithmetic T, std::endian endian = std::endian::native>
    binary_istream& read(T& val) {
        val = read<T>();

        return *this;
    }

    void set_bitwise(bool bitwise);
    bool get_bit();

    // Read a fixed-size integer value
    template <size_t bits>
    auto read() {
        min_uint_t<bits> val = 0;

        for (size_t i = 0; i < bits; ++i) {
            if (get_bit()) {
                val |= (1ui64 << i);
            }
        }

        return val;
    }

    // Read a fixed-size runtime integer value
    uintmax_t read_bits(size_t bits);

    protected:
    // Null constructor for base classes that manually set the file pointer
    binary_istream() = default;

    std::shared_ptr<std::istream> file;
    mutable std::mutex mutex;

    std::unique_ptr<std::streambuf> streambuf;

    private:
    bool _m_bitwise { }; // Whether we are writing bitwise data, this must be false
                         // before next non-bitwise read

    uint8_t _m_bit_buffer { };
    uint8_t _m_bit_buffer_size { };

    static_assert(CHAR_BIT == 8);
};

using istream_ptr = std::shared_ptr<binary_istream>;

class binary_ostream {
    public:
    using pos_type = std::istream::pos_type;
    using seekdir = std::istream::seekdir;

    // Open a file for writing
    explicit binary_ostream(const std::filesystem::path& path);

    // Take ownership of an existing std::ostream
    explicit binary_ostream(const std::shared_ptr<std::ostream>& stream);

    explicit binary_ostream(binary_ostream&& other) noexcept;
    explicit binary_ostream(const binary_ostream& other) = delete;

    virtual ~binary_ostream();

    virtual binary_ostream& write(const char* buf, std::streamsize size);

    virtual pos_type tellp() const;

    void set_bitwise(bool bitwise);
    bool bitwise() const;

    binary_ostream& put_bit(bool value);
    binary_ostream& flush_bits();

    // Write a POD value array
    template <concepts::pod T>
    binary_ostream& write(const T* buf, std::streamsize bytes) {
        return write(reinterpret_cast<const char*>(buf), bytes);
    }
    
    // Write a POD value array
    template <concepts::pod T, size_t size>
    binary_ostream& write(T(&buf)[size]) {
        return write(buf, size * sizeof(T));
    }

    // Write a single POD value
    template <concepts::pod T>
    binary_ostream& write(T val) {
        return write(&val, sizeof(T));
    }

    template <concepts::readable_container Container>
    binary_ostream& write(const Container& buf) {
        return write(buf.data(), buf.size() * sizeof(Container::value_type));
    }

    // Write a fixed-size integer value
    template <size_t bits>
    binary_ostream& write(min_uint_t<bits> val) {
        ASSERT(_m_bitwise);
        if (bits >= (bit_buffer_limit - _m_bit_buffer_size)) {
            size_t bits_to_put = bit_buffer_limit - _m_bit_buffer_size;
            _m_bit_buffer |= (static_cast<uint64_t>(val & ((1ui64 << bits_to_put) - 1))
                                    << _m_bit_buffer_size);
            flush_bits();
            val >>= bits_to_put;

            if (bits_to_put != bits) {
                _m_bit_buffer = static_cast<uint64_t>(val);
                _m_bit_buffer_size = (bits - bits_to_put);
            }

            
            return *this;
        }

        // There's enough place to emplace the entire value
        _m_bit_buffer |= (static_cast<uint64_t>(val) << _m_bit_buffer_size);
        _m_bit_buffer_size += bits;
     
        return *this;
    }

    binary_ostream& write_bits(uintmax_t val, size_t bits);

    protected:
    std::shared_ptr<std::ostream> file;
    mutable std::mutex mutex;

    private:
    bool _m_bitwise { }; // Whether we are writing bitwise data, this must be false
                         // before next non-bitwise read

    uint64_t _m_bit_buffer { };
    size_t _m_bit_buffer_size { };
    static constexpr size_t bit_buffer_limit = sizeof(_m_bit_buffer) * CHAR_BIT;

    static_assert(CHAR_BIT == 8);
};

using ostream_ptr = std::shared_ptr<binary_ostream>;

class binary_iostream : public binary_istream, public binary_ostream {
    public:
    explicit binary_iostream(const std::shared_ptr<std::iostream>& stream);

    std::iostream* stream() const;

    private:
    std::shared_ptr<std::iostream> _m_stream;
};

using iostream_ptr = std::shared_ptr<binary_iostream>;

template <typename T>
class bitwise_lock {
    T& _m_object;
    public:
    explicit bitwise_lock(T& object) : _m_object { object } {
        if constexpr (concepts::smart_pointer<T>) {
            _m_object.get()->set_bitwise(true);
        } else if constexpr (concepts::pointer<T>) {
            _m_object->set_bitwise(true);
        } else {
            _m_object.set_bitwise(true);
        }
    }

    ~bitwise_lock() {
        if constexpr (concepts::smart_pointer<T>) {
            _m_object.get()->set_bitwise(false);
        } else if constexpr (concepts::pointer<T>) {
            _m_object->set_bitwise(false);
        } else {
            _m_object.set_bitwise(false);
        }
    }
};

