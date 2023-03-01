// SPDX-License-Identifier: LGPL-3.0-or-later

#include "util/cripak_utf.hpp"

#include <iostream>

#include <fmt/ostream.h>

#include "util/file_stream.hpp"
#include "util/exceptions.hpp"

bool utf_table::field::has_value() const {
    return static_cast<bool>(flags & utf_flags::const_val)
        || static_cast<bool>(flags & utf_flags::row_val);
}

utf_table::utf_table(file_stream& fs)
    : _fs { fs }
    , _start { fs.tell() } {
    std::array<char, 4> fourcc;
    if (_fs.read(fourcc) != fourcc.size()) {
        throw io_error("unexpected EOF while reading @UTF");
    }

    if (fourcc != std::array { '@', 'U', 'T', 'F' }) {
        throw decode_error("unexpected magic number: expected '@UTF', got \"{} {} {} {}\"",
            fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
    }

    _header = {
        .table_size = _fs.read_u32be(),

        /* These offsets are from the start of the header, excluding the first 8 bytes, so translate to absolute */
        .rows_start     = _start + 8l + _fs.read_u32be(),
        .strings_start  = _start + 8l + _fs.read_u32be(),
        .data_start     = _start + 8l + _fs.read_u32be(),
        
        .table_name     = _fs.read_u32be(),
        .field_count    = _fs.read_u16be(),
        .row_align      = _fs.read_u16be(),
        .row_count      = _fs.read_u32be(),
    };

    if ((_start + 8 + _header.table_size) > _fs.size()) {
        throw decode_error("@UTF table truncated");
    }

    _fields.resize(_header.field_count);
    _rows.resize(_header.row_count);

    auto read_field = [this](utf_flags flags) -> utf_value {
        using enum utf_flags;
        switch (flags & utf_flags::type_mask) {
            case u8:  return _fs.read_u8();
            case s8:  return _fs.read_s8();
            case u16: return _fs.read_u16be();
            case s16: return _fs.read_s16be();
            case u32: return _fs.read_u32be();
            case s32: return _fs.read_s32be();
            case u64: return _fs.read_u64be();
            case s64: return _fs.read_u64be();
            case f32: return _fs.read_f32be();
            case f64: return _fs.read_f64be();
            case str: {
                /* Read offset relative to string table, seek, read, and seek back */
                uint32_t val_offset = _fs.read_u32be();
                std::streampos cur = _fs.tell();

                _fs.seek(_header.strings_start + val_offset);
                std::string res =  _fs.read_cstring();
                _fs.seek(cur);
                
                return res;
            }
            case dat: {
                /* Read offset relative to data table, read total size, seek, read, and seek back */
                uint32_t val_offset = _fs.read_u32be();
                uint32_t val_size = _fs.read_u32be();
                std::streampos cur = _fs.tell();

                _fs.seek(_header.data_start + val_offset);

                std::vector<std::byte> data(val_size);
                if (_fs.read(data) != val_size) {
                    throw io_error("unexpected EOF while reading data field");
                }

                _fs.seek(cur);
                return data;
            }
            default: break;
        }

        return {};
    };

    for (field& f : _fields) {
        f.flags = static_cast<utf_flags>(_fs.getb());

        /* If a name is attached to this field, read it from the string table */
        if ((f.flags & utf_flags::named) == utf_flags::named) {
            uint32_t name_offset = _fs.read_u32be();

            std::streampos cur = _fs.tell();

            _fs.seek(_header.strings_start + name_offset);
            f.name = _fs.read_cstring();
            _fs.seek(cur);
        }

        /* A constant value is stored just like a normal value */
        if ((f.flags & utf_flags::const_val) == utf_flags::const_val) {
            f.const_value = read_field(f.flags);
        }
    }

    _fs.seek(_header.rows_start);

    // TODO intellisense complains, prefer a span when this is fixed
    for (std::vector<utf_value>& row : _rows) {
        row.resize(_header.field_count);

        for (uint16_t i = 0; i < _header.field_count; ++i) {
            /* Read all fields for every row */
            field& f = _fields[i];

            /* Copy constant value if applicable */
            if (!std::holds_alternative<std::monostate>(f.const_value)) {
                row[i] = f.const_value;

            } else if ((f.flags & utf_flags::row_val) == utf_flags::row_val) {
                /* Or read the field value for this row */
                row[i] = read_field(f.flags);
            }
        }
    }
}

uint32_t utf_table::row_count() const {
    return _header.row_count;
}

std::span<const utf_table::field> utf_table::fields() const {
    return { _fields.begin(), _fields.end() };
}

bool utf_table::has_field(std::string_view name) const {
    for (const field& f : _fields) {
        if (f.name == name) {
            return true;
        }
    }

    return false;
}

uint16_t utf_table::field_index(std::string_view name) const {
    for (uint16_t i = 0; i < _fields.size(); ++i) {
        if (_fields[i].name == name) {
            return i;
        }
    }

    throw std::out_of_range("field not present");
}

const utf_table::field& utf_table::get_field(std::string_view name) const {
    return _fields[field_index(name)];
}

const utf_table::utf_value& utf_table::get(uint32_t row, std::string_view name) const {
    return _rows[row][field_index(name)];
}

utf_table::utf_value& utf_table::get(uint32_t row, std::string_view name) {
    return _rows[row][field_index(name)];
}
