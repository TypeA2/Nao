// SPDX-License-Identifier: LGPL-3.0-or-later

#include "util/cripak_utf.hpp"

#include <iostream>

#include <fmt/ostream.h>

#include "util/file_stream.hpp"
#include "util/exceptions.hpp"

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

    auto read_field = [this](utf_flags flags, utf_value& dest) {
        using enum utf_flags;
        switch (flags & utf_flags::flag_mask) {
            case u8:  dest = _fs.read_u8();    break;
            case s8:  dest = _fs.read_s8();    break;
            case u16: dest = _fs.read_u16be(); break;
            case s16: dest = _fs.read_s16be(); break;
            case u32: dest = _fs.read_u32be(); break;
            case s32: dest = _fs.read_s32be(); break;
            case u64: dest = _fs.read_u64be(); break;
            case s64: dest = _fs.read_u64be(); break;
            case f32: dest = _fs.read_f32be(); break;
            case f64: dest = _fs.read_f64be(); break;
            case str: {
                uint32_t val_offset = _fs.read_u32be();
                std::streampos cur = _fs.tell();

                _fs.seek(_header.strings_start + val_offset);

                dest = _fs.read_cstring();

                _fs.seek(cur);
                break;
            }
            case dat: {
                uint32_t val_offset = _fs.read_u32be();
                uint32_t val_size = _fs.read_u32be();
                std::streampos cur = _fs.tell();

                _fs.seek(_header.data_start + val_offset);

                std::vector<std::byte> data(val_size);
                if (_fs.read(data) != val_size) {
                    throw io_error("unexpected EOF while reading data field");
                }

                dest = data;

                _fs.seek(cur);
                break;
            }
            default: break;
        }
    };

    for (field& f : _fields) {
        f.flags = static_cast<utf_flags>(_fs.getb());

        if ((f.flags & utf_flags::named) == utf_flags::named) {
            f.name_offset = _fs.read_u32be();

            std::streampos cur = _fs.tell();

            _fs.seek(_header.strings_start + f.name_offset);

            f.name = _fs.read_cstring();

            _fs.seek(cur);
        }

        if ((f.flags & utf_flags::const_val) == utf_flags::const_val) {
            read_field(f.flags, f.const_value);
        }
    }

    for (field& f :  _fields) {
        fmt::print(std::cerr, "Field: flags={:x}, name={}, current idx={}\n", int(f.flags), f.name, f.const_value.index());
    }
}
