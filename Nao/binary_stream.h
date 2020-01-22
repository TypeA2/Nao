#pragma once

#include <istream>

class binary_stream {
    public:
    static_assert(std::endian::native == std::endian::little);

    using stream = std::shared_ptr<std::istream>;

    using pos_type = std::istream::pos_type;

    using seekdir = std::istream::seekdir;

    explicit binary_stream(stream s);

    virtual ~binary_stream() = default;

    virtual std::streampos tellg() const;
    virtual binary_stream& seekg(pos_type pos, seekdir dir);
    virtual bool eof() const;
    virtual binary_stream& ignore(std::streamsize max,
        std::istream::int_type delim = std::istream::traits_type::eof());

    // Seek with std::ios::cur
    binary_stream& rseek(pos_type pos);

    virtual binary_stream& read(char* buf, std::streamsize count);

    template <typename Container>
    binary_stream& read(Container& buf) {
        return read(buf.data(), buf.size());
    }

    template <typename T, size_t size>
    std::enable_if_t<std::is_arithmetic_v<T>, binary_stream&>
        read(T (& buf)[size]) {
        return read(buf, size);
    }

    template <typename T, std::endian endian = std::endian::native>
    std::enable_if_t<std::is_arithmetic_v<T>, T>
        read() {
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

    protected:
    stream file;
};

