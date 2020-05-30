#include "wwriff.h"
#include "byte_array_streambuf.h"

#include "riff.h"
#include "resource.h"

#include "ogg_stream.h"
#include "vorbis_encoder.h"

#include <nao/logging.h>

namespace wwriff {
    // libvorbis
    int ilog(unsigned int v) {
        int ret = 0;
        while (v) {
            ++ret;
            v >>= 1;
        }
        return ret;
    }

    long book_maptype1_quantvals(long entries, long dim) {
        if (entries < 1) {
            return(0);
        }
        long vals = static_cast<long>(floor(pow(entries, 1.f / static_cast<float>(dim))));

        /* the above *should* be reliable, but we'll not assume that FP is
           ever reliable when bitstream sync is at stake; verify via integer
           means that vals really is the greatest value of dim for which
           vals^b->bim <= b->entries */
           /* treat the above as an initial guess */
        if (vals < 1) {
            vals = 1;
        }

        while (true) {
            long acc = 1;
            long acc1 = 1;
            int i;
            for (i = 0; i < dim; ++i) {
                if (entries / vals < acc) {
                    break;
                }

                acc *= vals;

                if (std::numeric_limits<long>::max() / (vals + 1) < acc1) {
                    acc1 = std::numeric_limits<long>::max();
                } else {
                    acc1 *= vals + 1;
                }
            }

            if (i >= dim && acc <= entries && acc1 > entries) {
                return vals;
            }

            if (i < dim || acc > entries) {
                --vals;
            } else {
                ++vals;
            }
        }
    }



    bool wwriff_to_ogg(const istream_ptr& in, const ostream_ptr& out) {
        auto start = std::chrono::steady_clock::now();

        wwriff_converter conv(in);
        if (!conv.parse()) {
            return false;
        }
        
        if (!conv.convert(out)) {
            return false;
        }

        auto end = std::chrono::steady_clock::now();
        nao::coutln("Converted in", (end - start).count() / 1e6, "ms");

        return true;
    }

}

vorbis_packet::vorbis_packet(const istream_ptr& stream, std::streamoff offset, bool no_granule)
    : _m_offset { offset }, _m_no_granule { no_granule } {
    stream->seekg(offset);
    _m_size = stream->read<uint16_t>();
    if (!no_granule) {
        stream->read(_m_granule);
    }
}

std::streamsize vorbis_packet::header_size() const {
    return _m_no_granule ? 2 : 6;
}

std::streamoff vorbis_packet::this_offset() const {
    return _m_offset + header_size();
}

std::streamoff vorbis_packet::next_offset() const {
    return _m_offset + 2 + _m_size;
}

std::streamsize vorbis_packet::size() const {
    return _m_size;
}

uint32_t vorbis_packet::granule() const {
    return _m_granule;
}

codebook_library::codebook_library(const istream_ptr& codebook) : stream { codebook } {
    stream->seekg(0, std::ios::end);
    std::streamoff filesize = stream->tellg();
    stream->seekg(-4, std::ios::end);

    uint32_t offset = stream->read<uint32_t>();
    _m_codebook_count = static_cast<uint32_t>(filesize - offset) / 4;

    stream->seekg(0);

    _m_data.resize(offset);
    stream->read(_m_data.data(), offset);

    _m_offsets.resize(_m_codebook_count);

    for (uint32_t i = 0; i < _m_codebook_count; ++i) {
        auto _offset = stream->read<uint32_t>();
        _m_offsets[i] = _offset;
    }

}

const char* codebook_library::get_codebook(uint32_t id) const {
    if (id >= (_m_codebook_count - 1)) {
        return nullptr;
    }

    return _m_data.data() + _m_offsets[id];
}

std::streamsize codebook_library::get_size(uint32_t id) const {
    if (id >= (_m_codebook_count - 1)) {
        return -1;
    }

    return _m_offsets[id + 1ui64] - _m_offsets[id];
}

bool codebook_library::rebuild(uint32_t id, binary_ostream& os) const {
    auto cb = get_codebook(id);
    std::streamoff size = get_size(id);

    if (size == 0 || size == -1) {
        return false;
    }

    binary_istream in(std::make_unique<byte_array_streambuf>(cb, size));

    bitwise_lock lock { in };
    rebuild(in, os);

    return true;
}

void codebook_library::rebuild(binary_istream& in, binary_ostream& out) {
    auto dimensions = in.read<4>();
    auto entries = in.read<14>();

    out.write<8>('B')
        .write<8>('C')
        .write<8>('V')
        .write<16>(dimensions)
        .write<24>(entries);

    auto ordered = in.read<1>();
    out.write<1>(ordered);

    if (ordered != 0) {
        out.write<5>(in.read<5>()); // Initial

        uintmax_t current = 0;
        while (current < entries) {
            auto bits = wwriff::ilog(static_cast<int>(entries - current));
            auto number = in.read_bits(bits);
            out.write_bits(number, bits);

            current += number;
        }

        ASSERT(current <= entries);
    } else {
        auto codeword_length_length = in.read<3>();
        auto sparse = in.read<1>();

        ASSERT(codeword_length_length != 0 && codeword_length_length <= 5);

        out.write<1>(sparse);

        for (uint16_t i = 0; i < entries; ++i) {
            if (sparse != 0) {
                auto present = in.read<1>();
                out.write<1>(present);

                if (present == 0) {
                    continue;
                }
            }

            out.write<5>(in.read_bits(codeword_length_length) & 0b11111);
        }
    }

    // Lookup
    auto type = in.read<1>();
    out.write<4>(type);

    switch (type) {
        case 0: break; // no lookup
        case 1: {
            out.write<32>(in.read<32>()) // min
                .write<32>(in.read<32>()); // max

            auto val_length = in.read<4>();
            out.write<4>(val_length)
                .write<1>(in.read<1>()); // Sequence flag

            uint32_t quantvals = wwriff::book_maptype1_quantvals(entries, dimensions);
            for (uint32_t i = 0; i < quantvals; ++i) {
                auto val = in.read_bits(val_length + 1ui64);
                out.write_bits(val, val_length + 1ui64);
            }

            break;
        }

        case 2:
        default: ASSERT(false);
    }


}

wwriff_converter::wwriff_converter(const istream_ptr& in) : in { in } {
    
}

bool wwriff_converter::parse() {
    CHECK(_validate_header());
    CHECK(_gather_chunks());
    CHECK(_validate_chunks());
    CHECK(_parse_chunks());

    _parsed = true;

    return true;
}

bool wwriff_converter::convert(const ostream_ptr& out) {
    CHECK(_parsed);

    ogg_stream os(out, 1);
    vorbis_encoder vc;

    CHECK(_write_header(os, vc));
    CHECK(_write_comment(os, vc));
    CHECK(_write_setup(os, vc));
    CHECK(_write_audio(os, vc));

    return true;
}

bool wwriff_converter::_validate_header() {
    riff_header hdr;
    in->read(&hdr, sizeof(hdr));

    CHECK(in->gcount() == sizeof(hdr));
    CHECK(std::string(hdr.header, 4) == "RIFF");

    wave_chunk wave;
    in->read(&wave, sizeof(wave));
    CHECK(in->gcount() == sizeof(wave));
    CHECK(std::string(wave.wave, 4) == "WAVE");

    _riff_size = hdr.size + 8;

    return true;
}

bool wwriff_converter::_gather_chunks() {
    std::streamoff current_offset = in->tellg();

    riff_header chunk;
    while (current_offset < _riff_size) {
        in->seekg(current_offset);

        // Make sure there's space for the header
        CHECK((current_offset + 8) <= _riff_size);

        in->read(&chunk, sizeof(chunk));
        CHECK(in->gcount() == sizeof(chunk));

        // Check all chunks
        for (size_t i = 0; i < CHUNK_COUNT; ++i) {
            if (std::string(chunk.header, 4) == chunk_map[i]) {
                _chunks[i] = {
                    .offset = current_offset + 8,
                    .size = chunk.size
                };

                _chunks_found[i] = true;
                break;
            }
        }

        // Advance by the size of the chunk
        current_offset = current_offset + sizeof(chunk) + chunk.size;
    }

    CHECK(current_offset <= _riff_size);

    return true;
}

bool wwriff_converter::_validate_chunks() {
    CHECK(_chunks_found[FMT] && _chunks_found[DATA]);

    CHECK(!_chunks_found[VORB] && _chunks[FMT].size == 66);

    CHECK(!_chunks_found[VORB] &&
        !(_chunks[FMT].size == 40 || _chunks[FMT].size == 24 || _chunks[FMT].size == 18));

    // Fake vorb
    if (!_chunks_found[VORB] && _chunks[FMT].size == 66) {
        _chunks[VORB].offset = _chunks[FMT].offset + 24;
    }

    return true;
}

bool wwriff_converter::_parse_chunks() {
    CHECK(_parse_fmt());
    CHECK(_parse_cue());
    CHECK(_parse_smpl());
    CHECK(_parse_vorb());

    return true;
}

bool wwriff_converter::_parse_fmt() {
    wwriff_chunk& fmt = _chunks[FMT];

    in->seekg(fmt.offset);

    fmt_chunk _fmt;
    in->read(&_fmt, sizeof(_fmt));
    CHECK(in->gcount() == sizeof(_fmt));

    CHECK(_fmt.format == 0xFFFF);
    CHECK(_fmt.align == 0);
    CHECK(_fmt.bits == 0);

    _channels = _fmt.channels;
    _rate = _fmt.rate;
    _bitrate = _fmt.byte_rate * 8;

    // Extra format length
    CHECK(in->read<uint16_t>() == fmt.size - 18);

    uint16_t ext_unk = 0;
    uint32_t subtype = 0;
    if ((fmt.size - 18) >= 2) {
        in->read(ext_unk);
        if ((fmt.size - 18) >= 6) {
            in->read(subtype);
        }
    }

    if (fmt.size == 40) {
        static constexpr uint8_t magic_buf[16] {
            1,   0,  0,   0,
            0,   0,  16,  0,
            128, 0,  0,   170,
            0,   56, 155, 113
        };

        uint8_t magic[16];
        in->read(magic);

        CHECK(memcmp(magic, magic_buf, sizeof(magic_buf)) == 0);
    }

    return true;
}

bool wwriff_converter::_parse_cue() {
    // Read cue
    if (_chunks_found[CUE]) {
        in->seekg(_chunks[CUE].offset);
        in->read(_cue_count);
    }

    return true;
}

bool wwriff_converter::_parse_smpl() {
    // smpl
    if (_chunks_found[SMPL]) {
        in->seekg(_chunks[SMPL].offset + 28);
        in->read(_loop_count);

        CHECK(_loop_count == 1);

        in->seekg(_chunks[SMPL].offset + 44);
        in->read(_loop_start);
        in->read(_loop_end);
    }

    return true;
}

bool wwriff_converter::_parse_vorb() {
    wwriff_chunk& vorb = _chunks[VORB];

    // vorb
    CHECK(vorb.size == 0 || vorb.size == 40 || vorb.size == 42
        || vorb.size == 44 || vorb.size == 50 || vorb.size == 52);
    
    in->seekg(vorb.offset);

    uint32_t sample_count = in->read<uint32_t>();

    switch (vorb.size) {
        case 0:
        case 42: {
            in->seekg(vorb.offset + 4);
            uint32_t mod_signal = in->read<uint32_t>();

            if (mod_signal != 74 && mod_signal != 75 &&
                mod_signal != 105 && mod_signal != 112) {
                _mod_packets = true;
            }

            in->seekg(vorb.offset + 16);
            break;
        }

        default: return false; // Has granule, not supported
    }

    _setup_offset = in->read<uint32_t>();
    _audio_offset = in->read<uint32_t>();

    switch (vorb.size) {
        case 0:
        case 42:
            in->seekg(vorb.offset + 36);
            break;

        case 50:
        case 52:
            in->seekg(vorb.offset + 44);
            break;

        default: break;
    }

    uint32_t uid = 0;
    switch (vorb.size) {
        case 40:
        case 44: return false; // not supported

        case 0:  case 42: case 50: case 52:
            in->read(uid);
            in->read(_blocksize_0_pow);
            in->read(_blocksize_1_pow);
            break;

        default: break;
    }

    if (_loop_count) {
        if (_loop_end == 0) {
            _loop_end = sample_count;
        } else {
            ++_loop_end;
        }

        CHECK(_loop_start < sample_count && _loop_end <= sample_count && _loop_start <= _loop_end);
    }

    return true;
}

bool wwriff_converter::_write_header(ogg_stream& os, vorbis_encoder& vc) const {
    auto ss = std::make_shared<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary);
    binary_ostream temp(ss);

    temp.write("\x01vorbis", 7);

    {
        bitwise_lock lock { temp };

        temp.write<32>(0) // version
            .write<8>(_channels) // channels
            .write<32>(_rate) // sample rate
            .write<32>(0) // max bitrate
            .write<32>(_bitrate) // bitrate
            .write<32>(0) // min bitrate
            .write<4>(_blocksize_0_pow) // blocksize0
            .write<4>(_blocksize_1_pow) // blocksize1
            .write<1>(1); // framing
    }

    auto packet_data = ss->str();

    ogg_packet packet = os.packet(packet_data.data(), packet_data.size());
    os.packetin(packet);
    os.flush();

    CHECK(vc.headerin(packet));

    return true;
}

bool wwriff_converter::_write_comment(ogg_stream& os, vorbis_encoder& vc) const {
    auto ss = std::make_shared<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary);
    binary_ostream temp(ss);

    temp.write("\x03vorbis", 7);

    {
        bitwise_lock lock { temp };

        static constexpr std::string_view vendor = "ww2ogg Nao implementation";

        temp.write<32>(static_cast<uint32_t>(vendor.size()));

        for (char c : vendor) {
            if (!c) {
                break;
            }

            temp.write<8>(c);
        }

        if (_loop_count == 0) {
            // No comments
            temp.write<32>(0);
        } else {
            // 2 comments
            temp.write<32>(2);

            std::stringstream loop_ss;
            loop_ss << "LoopStart=" << _loop_start;

            std::string str = loop_ss.str();
            temp.write<32>(static_cast<uint32_t>(str.size()));
            for (char c : str) {
                temp.write<8>(c);
            }

            loop_ss.str("");
            loop_ss << "LoopEnd=" << _loop_end;

            str = loop_ss.str();
            temp.write<32>(static_cast<uint32_t>(str.size()));
            for (char c : str) {
                temp.write<8>(c);
            }
        }

        temp.write<1>(1); // Framing
    }

    auto packet_data = ss->str();

    ogg_packet packet = os.packet(packet_data.data(), packet_data.size());
    os.packetin(packet);
    os.pageout();
    
    CHECK(vc.headerin(packet));

    return true;
}

bool wwriff_converter::_write_setup(ogg_stream& os, vorbis_encoder& vc) {
    auto ss = std::make_shared<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary);
    binary_ostream temp(ss);

    temp.write("\x05vorbis", 7);

    vorbis_packet setup_packet(in, _chunks[DATA].offset + _setup_offset, true);
    in->seekg(setup_packet.this_offset());

    {
        bitwise_lock lock { in };
        bitwise_lock lock2 { temp };
        auto codebook_count_less1 = in->read<8>();

        _codebook_count = codebook_count_less1 + 1;
        
        temp.write<8>(codebook_count_less1);

        codebook_library cbl(std::make_shared<binary_istream>(IDR_PACKED_CODEBOOKS_AOTUV_603));

        for (uint32_t i = 0; i < _codebook_count; ++i) {
            CHECK(cbl.rebuild(in->read<10>(), temp));
        }

        // Time domain transforms
        temp.write<6>(0);
        temp.write<16>(0);

        CHECK(_write_floors(temp));
        CHECK(_write_residue(temp));
        CHECK(_write_mapping(temp));
        CHECK(_write_mode(temp));

        temp.write<1>(1); // framing
    }

    auto packet_data = ss->str();

    CHECK(setup_packet.next_offset() == (_chunks[DATA].offset + _audio_offset));

    ogg_packet packet = os.packet(packet_data.data(), packet_data.size());
    os.packetin(packet);
    os.flush();

    CHECK(vc.headerin(packet));

    return true;
}

bool wwriff_converter::_write_audio(ogg_stream& os, vorbis_encoder& vc) const {
    auto offset = _chunks[DATA].offset + _audio_offset;

    // Temporary buffer to hold packet data
    std::vector<uint64_t> buf;

    long last_bs = 0;
    int64_t granulepos = 0;

    auto ss = std::make_shared<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary);
    binary_ostream temp(ss);

    const wwriff_chunk& data = _chunks[DATA];

    bool prev_flag = false;

    while (offset < (data.offset + data.size)) {
        {
            vorbis_packet packet { in, offset, true };

            CHECK((offset + packet.header_size()) <= (data.offset + data.size));

            offset = packet.this_offset();

            in->seekg(offset);

            bitwise_lock lock { temp };

            if (_mod_packets) {
                CHECK(!_mode_flag.empty());

                temp.write<1>(0); // type audio

                size_t remainder;
                uintmax_t mode_number;
                {
                    bitwise_lock lock1 { in };
                    mode_number = in->read_bits(_mode_bits);
                    temp.write_bits(mode_number, _mode_bits);

                    remainder = in->read_bits(8 - _mode_bits);
                }

                if (_mode_flag[mode_number]) {
                    std::streamoff next_offset = packet.next_offset();
                    in->seekg(next_offset);

                    bool next_flag = false;

                    if ((next_offset + packet.header_size()) <= (data.offset + data.size)) {
                        vorbis_packet next_packet(in, next_offset, true);

                        if (next_packet.size() > 0) {
                            in->seekg(next_packet.this_offset());

                            bitwise_lock lock1 { in };

                            next_flag = _mode_flag[in->read_bits(_mode_bits)];
                        }
                    }

                    temp.write<1>(prev_flag ? 1 : 0)
                        .write<1>(next_flag ? 1 : 0);

                    in->seekg(offset + 1);
                }

                prev_flag = _mode_flag[mode_number];

                temp.write_bits(remainder, 8 - _mode_bits);
            } else {
                temp.write<8>(in->read<uint8_t>());
            }

            size_t bytes = packet.size() - 1;
            buf.resize((bytes + (sizeof(uint64_t) - 1)) / sizeof(uint64_t));

            in->read(buf.data(), bytes);

            for (uint64_t c : buf) {
                if (bytes >= sizeof(uint64_t)) {
                    // At least 1 full value left
                    temp.write<sizeof(uint64_t) * CHAR_BIT>(c);
                    bytes -= sizeof(uint64_t);
                } else {
                    temp.write_bits(c, bytes * CHAR_BIT);
                }

            }

            offset = packet.next_offset();
        }

        auto packet_data = ss->str();

        ogg_packet packet = os.packet(packet_data.data(), packet_data.size());
        long bs = vc.blocksize(packet);

        CHECK(bs > 0);
        if (last_bs > 0) {
            granulepos += (last_bs + bs) / 4;
        }

        last_bs = bs;
        packet.granulepos = granulepos;

        if (offset >= (data.offset + data.size)) {
            packet.e_o_s = 1;
        }

        os.packetin(packet);
        os.pageout();

        ss->str("");
    }

    return true;
}

bool wwriff_converter::_write_floors(binary_ostream& out) {
    // Floors
    auto floor_count_less1 = in->read<6>();

    _floor_count = floor_count_less1 + 1;

    out.write<6>(floor_count_less1);

    for (uint32_t i = 0; i < _floor_count; ++i) {
        out.write<16>(1);

        auto floor1_partitions = in->read<5>();
        out.write<5>(floor1_partitions);

        std::vector<uint8_t> floor1_partition_class_list(floor1_partitions);
        uint8_t max_class = 0;
        for (uint8_t j = 0; j < floor1_partitions; ++j) {
            auto floor1_partition_class = in->read<4>();
            out.write<4>(floor1_partition_class);

            floor1_partition_class_list[j] = floor1_partition_class;

            if (floor1_partition_class > max_class) {
                max_class = floor1_partition_class;
            }
        }

        std::vector<uint8_t> floor1_class_dimension_list(max_class + 1ui64);
        for (uint8_t j = 0; j <= max_class; ++j) {
            auto class_dimension_less1 = in->read<3>();
            out.write<3>(class_dimension_less1);

            floor1_class_dimension_list[j] = class_dimension_less1 + 1;

            auto subclasses = in->read<2>();
            out.write<2>(subclasses);

            if (subclasses != 0) {
                auto master_book = in->read<8>();
                out.write<8>(master_book);

                CHECK(master_book < _codebook_count);
            }

            for (uint8_t k = 0; k < (1ui8 << subclasses); ++k) {
                auto subclass_book_plus1 = in->read<8>();
                out.write<8>(subclass_book_plus1);

                int16_t subclass_book = static_cast<int16_t>(subclass_book_plus1) - 1;
                if (subclass_book >= 0 && static_cast<uint16_t>(subclass_book) >= _codebook_count) {
                    return false;
                }
            }
        }

        out.write<2>(in->read<2>()); // floor1_multiplier_less1

        auto rangebits = in->read<4>();
        out.write<4>(rangebits);

        for (uint8_t j = 0; j < floor1_partitions; ++j) {
            for (uint8_t k = 0; k < floor1_class_dimension_list[floor1_partition_class_list[j]]; ++k) {
                out.write_bits(in->read_bits(rangebits), rangebits);
            }
        }
    }

    return true;
}

bool wwriff_converter::_write_residue(binary_ostream& out) {
    // Residue
    auto residue_count_less1 = in->read<6>();
    out.write<6>(residue_count_less1);

    _residue_count = residue_count_less1 + 1;

    for (uint32_t i = 0; i < _residue_count; ++i) {
        auto type = in->read<2>();
        out.write<16>(type);

        CHECK(type <= 2);

        auto begin = in->read<24>();
        auto end = in->read<24>();
        auto residue_partition_size_less1 = in->read<24>();
        auto residue_classifications_less1 = in->read<6>();
        auto residue_classbook = in->read<8>();

        uint8_t residue_classifications = residue_classifications_less1 + 1;

        out.write<24>(begin)
            .write<24>(end)
            .write<24>(residue_partition_size_less1)
            .write<6>(residue_classifications_less1)
            .write<8>(residue_classbook);

        CHECK(residue_classbook < _codebook_count);

        std::vector<uint16_t> cascade(residue_classifications);

        for (uint8_t j = 0; j < residue_classifications; ++j) {
            auto low_bits = in->read<3>();
            out.write<3>(low_bits);

            cascade[j] = low_bits;

            auto flag = in->read<1>();
            out.write<1>(flag);

            if (flag != 0) {
                auto high_bits = in->read<5>();
                out.write<5>(high_bits);
                cascade[j] |= (high_bits << 3);
            }
        }

        for (uint8_t j = 0; j < residue_classifications; ++j) {
            for (uint8_t k = 0; k < 8; ++k) {
                if (cascade[j] & (1 << k)) {
                    auto book = in->read<8>();
                    out.write<8>(book);

                    CHECK(book < _codebook_count);
                }
            }
        }
    }

    return true;
}

bool wwriff_converter::_write_mapping(binary_ostream& out) {
    // Mapping
    auto mapping_count_less1 = in->read<6>();
    out.write<6>(mapping_count_less1);

    _mapping_count = mapping_count_less1 + 1;

    for (uint32_t i = 0; i < _mapping_count; ++i) {
        out.write<16>(0); // mapping type

        auto flag = in->read<1>();
        out.write<1>(flag);

        uint8_t submaps = 1;
        if (flag != 0) {
            auto submaps_less1 = in->read<4>();
            out.write<4>(submaps_less1);

            submaps = submaps_less1 + 1;
        }

        auto square_polar_flag = in->read<1>();
        out.write<1>(square_polar_flag);

        if (square_polar_flag != 0) {
            auto coupling_steps_less1 = in->read<8>();
            out.write<8>(coupling_steps_less1);

            uint16_t coupling_steps = coupling_steps_less1 + 1;

            for (uint16_t j = 0; j < coupling_steps; ++j) {
                const auto bits = wwriff::ilog(_channels - 1);
                auto magnitude = in->read_bits(bits);
                auto angle = in->read_bits(bits);

                out.write_bits(magnitude, bits)
                    .write_bits(angle, bits);
                CHECK(angle != magnitude && magnitude < _channels && angle < _channels);
            }
        }

        auto reserved = in->read<2>();
        out.write<2>(reserved);
        CHECK(reserved == 0);

        if (submaps > 1) {
            for (uint32_t j = 0; j < _channels; ++j) {
                auto mux = in->read<4>();
                out.write<4>(mux);

                CHECK(mux < submaps);
            }
        }

        for (uint16_t j = 0; j < submaps; ++j) {
            out.write<8>(in->read<8>()); // config

            auto floor_number = in->read<8>();
            out.write<8>(floor_number);

            CHECK(floor_number < _floor_count);

            auto residue_number = in->read<8>();
            out.write<8>(residue_number);

            CHECK(residue_number < _residue_count);
        }
    }

    return true;
}

bool wwriff_converter::_write_mode(binary_ostream& out) {
    auto mode_count_less1 = in->read<6>();
    out.write<6>(mode_count_less1);

    uint8_t mode_count = mode_count_less1 + 1;

    _mode_flag.resize(mode_count);
    _mode_bits = wwriff::ilog(mode_count - 1);

    for (uint8_t i = 0; i < mode_count; ++i) {
        auto flag = in->read<1>();
        out.write<1>(flag);

        _mode_flag[i] = flag != 0;

        out.write<16>(0) // window type
            .write<16>(0); // transform type

        auto mapping = in->read<8>();
        out.write<8>(mapping);

        CHECK(mapping < _mapping_count);
    }

    return true;
}

std::string wwriff_converter::chunk_map[CHUNK_COUNT] {
    "fmt ", "cue ", "LIST", "smpl", "vorb", "data"
};
