#include "encoding.h"

#include <unordered_map>

namespace detail {
    const char* name(nao::text_encoding enc) {
        static std::unordered_map<nao::text_encoding, const char*> names{
            { nao::text_encoding::ASCII,         "ASCII"     },
            { nao::text_encoding::Shift_JIS,     "SHIFT_JIS" },
            { nao::text_encoding::UTF8,          "UTF-8" },
            { nao::text_encoding::UCS2,          "UCS-2" },
            { nao::text_encoding::UCS2BE,        "UCS-2BE" },
            { nao::text_encoding::UCS2LE,        "UCS-2LE" },
            { nao::text_encoding::UCS4,          "UCS-4" },
            { nao::text_encoding::UCS4LE,        "UCS-4BE" },
            { nao::text_encoding::UCS4BE,        "UCS-4LE" },
            { nao::text_encoding::UTF16,         "UTF-16" },
            { nao::text_encoding::UTF16BE,       "UTF-16BE" },
            { nao::text_encoding::UTF16LE,       "UTF-16LE" },
            { nao::text_encoding::UTF32,         "UTF-32" },
            { nao::text_encoding::UTF32BE,       "UTF-32BE" },
            { nao::text_encoding::UTF32LE,       "UTF-32LE" },
            { nao::text_encoding::UCS2_Internal, "UCS-2-INTERNAL" },
            { nao::text_encoding::UCS4_Internal, "UCS-4-INTERNAL" },
            { nao::text_encoding::Char,          "char" },
            { nao::text_encoding::WChar_t,       "wchar_t" },
        };

        return names.at(enc);
    }
}

namespace nao {
    std::string_view text_encoding_name(text_encoding t) {



        return detail::name(t);
    }


    text_converter::text_converter(text_encoding from, text_encoding to)
        : _from{ from }, _to{ to }
        , _cd{ iconv_open(detail::name(_to), detail::name(_from)) } {

    }


    text_converter::text_converter(const text_converter& other)
        : text_converter{ other._from, other._to } { }


    text_converter& text_converter::operator=(const text_converter& other) {
        _from = other._from;
        _to = other._to;

        if (_cd) {
            iconv_close(_cd);
        }

        _cd = iconv_open(detail::name(_to), detail::name(_from));
        return *this;
    }


    text_converter::text_converter(text_converter&& other) noexcept {
        *this = std::forward<text_converter>(other);
    }


    text_converter& text_converter::operator=(text_converter&& other) noexcept {
        _from = other._from;
        _to = other._to;

        if (_cd) {
            iconv_close(_cd);
        }

        _cd = std::exchange(other._cd, nullptr);
        return *this;
    }


    text_converter::~text_converter() {
        iconv_close(_cd);
    }


}