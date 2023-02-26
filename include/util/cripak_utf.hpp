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
        std::monostate,
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

        type_mask = 0x0F,

        named     = 0x10,
        const_val = 0x20,
        row_val   = 0x40,

        flag_mask = 0xF0,
    };

    struct field {
        utf_flags flags;
        std::string name;
        utf_value const_value;
    };

    private:
    file_stream& _fs;
    std::streamoff _start;
    header _header;
    std::vector<field> _fields;
    std::vector<std::vector<utf_value>> _rows;

    public:
    explicit utf_table(file_stream& fs);

    /**
     * @brief Retrieve the total number of rows
     * 
     * @return uint32_t 
     */
    [[nodiscard]] uint32_t rows() const;

    /**
     * @brief Query whether thist able has a specific field, by name
     * 
     * @param name 
     * @return true 
     * @return false 
     */
    [[nodiscard]] bool has_field(std::string_view name) const;

    /**
     * @brief Retrieve the index of the first field with the specified name
     * 
     * @param name 
     * @return uint16_t 
     */
    [[nodiscard]] uint16_t field_index(std::string_view name) const;

    /**
     * @brief Retrieve a value by row and field
     * 
     * @param row 
     * @param name 
     * @return const utf_value& 
     */
    [[nodiscard]] const utf_value& get(uint32_t row, std::string_view name) const;

    /**
     * @brief Retrieve a value by row and field
     * 
     * @param row 
     * @param name 
     * @return utf_value& 
     */
    [[nodiscard]] utf_value& get(uint32_t row, std::string_view name);

    /**
     * @brief Retrieve a specific value
     * 
     * @tparam T 
     * @param row 
     * @param name 
     * @return const T& 
     */
    template <typename T>
    [[nodiscard]] const T& get(uint32_t row, std::string_view name) const {
        return std::get<T>(get(row, name));
    }

    /**
     * @brief Retrieve a specific value
     * 
     * @tparam T 
     * @param row 
     * @param name 
     * @return T& 
     */
    template <typename T>
    [[nodiscard]] T& get(uint32_t row, std::string_view name) {
        return std::get<T>(get(row, name));
    }
};

#endif /* CRIPAK_UTF_HPP */
