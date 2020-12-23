#pragma once

#include <iconv.h>

#include <string>
#include <vector>

namespace nao {
    enum class text_encoding {
        ASCII,
        Shift_JIS,

        UTF8,
        UCS2,  UCS2BE,  UCS2LE,
        UCS4,  UCS4BE,  UCS4LE,
        UTF16, UTF16BE, UTF16LE,
        UTF32, UTF32BE, UTF32LE,

        UCS2_Internal, UCS4_Internal,

        Char, WChar_t
    };

    std::string_view text_encoding_name(text_encoding t);

    class text_converter {
        text_encoding _from;
        text_encoding _to;

        iconv_t _cd{};

        public:
        text_converter(text_encoding from, text_encoding to);

        text_converter(const text_converter& other);
        text_converter& operator=(const text_converter& other);

        text_converter(text_converter&& other) noexcept;
        text_converter& operator=(text_converter&& other) noexcept;

        ~text_converter();

        template <typename To, typename From>
        std::basic_string<To> convert(std::basic_string<From> data) const {
            char* inbuf = reinterpret_cast<char*>(data.data());
            size_t insize = data.size() * sizeof(From);

            std::vector<To> buf(data.size() + 1, To{});

            char* outbuf = reinterpret_cast<char*>(buf.data());
            size_t outbytes_left = buf.size() * sizeof(To);

            // Keep converting characters until we're done
            do {
                // Convert as many characters as possible
                size_t res = iconv(_cd, &inbuf, &insize, &outbuf, &outbytes_left);

                if (res == static_cast<size_t>(-1) && errno == E2BIG) {
                    // Need more space
                    size_t bytes_used = outbuf - reinterpret_cast<char*>(buf.data());

                    buf.resize(buf.size() * 2);
                    outbuf = reinterpret_cast<char*>(buf.data()) + bytes_used;
                    outbytes_left = (buf.size() * sizeof(To)) - bytes_used;
                }
            } while (insize > 0);

            return buf.data();
        }
    };
}
