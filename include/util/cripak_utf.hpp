// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CRIPAK_UTF_HPP
#define CRIPAK_UTF_HPP

#include <ios>
#include <vector>
#include <string>
#include <vector>
#include <variant>

#include <magic_enum.hpp>

using namespace magic_enum::bitwise_operators;

class file_stream;

/**
 * @brief An @UTF table consists of row filled with predefined fields of the corresponding types
 * 
 */
class utf_table {
    public:
    struct header {
        uint32_t table_size;
        std::streamoff rows_start;
        std::streamoff strings_start;
        std::streamoff data_start;
        uint32_t table_name;
        uint16_t field_count;
        uint16_t row_align;
        uint32_t row_count;
    };

    using utf_value = std::variant<
        uint8_t,  int8_t,
        uint16_t, int16_t,
        uint32_t, int32_t,
        uint64_t, int64_t,
        float,    double,
        std::string,
        std::vector<std::byte>
    >;

    enum class utf_flags : uint8_t {
        u8  = 0x00, s8  = 0x01,
        u16 = 0x02, s16 = 0x03,
        u32 = 0x04, s32 = 0x05,
        u64 = 0x06, s64 = 0x07,
        f32 = 0x08, f64 = 0x09,
        str = 0x0A, dat = 0x0B,

        named     = 0x10,
        const_val = 0x20,
        row_val   = 0x40,

        flag_mask = 0xF0,
    };

    struct field {
        utf_flags flags;

        uint32_t name_offset;
        std::string name;
        
        utf_value const_value;
    };

    private:
    file_stream& _fs;
    std::streamoff _start;
    header _header;
    std::vector<field> _fields;

    public:
    explicit utf_table(file_stream& fs);
};

#endif /* CRIPAK_UTF_HPP */
