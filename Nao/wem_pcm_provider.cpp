#include "wem_pcm_provider.h"

#include "riff.h"

#include "utils.h"

#include <ogg/ogg.h>

#include <random>

#include "frameworks.h"
#include "resource.h"

#include "byte_array_streambuf.h"
#include <vorbis/vorbisenc.h>

class vorbis_packet {
    public:
    explicit vorbis_packet(const istream_ptr& stream, std::streamoff offset, bool no_granule = false)
        : _m_offset { offset }, _m_no_granule { no_granule } {
        stream->seekg(offset);
        _m_size = stream->read<uint16_t>();
        if (!no_granule) {
            stream->read(_m_granule);
        }
    }

    // Granule means the header is larger
    std::streamsize header_size() const { return _m_no_granule ? 2 : 6; };
    std::streamoff this_offset() const { return _m_offset + header_size(); }
    std::streamoff next_offset() const { return _m_offset + 2 + _m_size; }
    std::streamsize size() const { return _m_size; }
    uint32_t granule() const { return _m_granule; }

    private:
    std::streamoff _m_offset;
    std::streamsize _m_size;
    uint32_t _m_granule { };
    bool _m_no_granule;
};

// libvorbis
static int ilog(unsigned int v) {
    int ret=0;
    while (v) {
        ret++;
        v>>=1;
    }
    return(ret);
}

static long _book_maptype1_quantvals(long entries, long dim) {
    long vals;
    if (entries < 1) {
        return(0);
    }
    vals=static_cast<long>(floor(pow(entries, 1.f / static_cast<float>(dim))));

    /* the above *should* be reliable, but we'll not assume that FP is
       ever reliable when bitstream sync is at stake; verify via integer
       means that vals really is the greatest value of dim for which
       vals^b->bim <= b->entries */
       /* treat the above as an initial guess */
    if (vals < 1) {
        vals=1;
    }
    while (true) {
        long acc=1;
        long acc1=1;
        int i;
        for (i=0; i < dim; i++) {
            if (entries / vals < acc)break;
            acc*=vals;
            if (LONG_MAX / (vals + 1) < acc1)acc1=LONG_MAX;
            else acc1*=vals + 1;
        }
        if (i >= dim && acc <= entries && acc1 > entries) {
            return(vals);
        } else {
            if (i<dim || acc>entries) {
                vals--;
            } else {
                vals++;
            }
        }
    }
}

class codebook_library {
    public:
    explicit codebook_library(const istream_ptr& codebook) : stream { codebook } {
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

        utils::coutln("parsed", _m_codebook_count, "codebooks, offset", offset);
    }

    const char* get_codebook(uint32_t id) const {
        if (id >= (_m_codebook_count - 1)) {
            return nullptr;
        }

        return _m_data.data() + _m_offsets[id];
    }

    std::streamsize get_size(uint32_t id) {
        if (id >= (_m_codebook_count - 1)) {
            return -1;
        }

        return _m_offsets[id + 1ui64] - _m_offsets[id];
    }

    bool rebuild(uint32_t id, binary_ostream& os) {
        auto cb = get_codebook(id);
        std::streamoff size = get_size(id);
        
        if (size == 0 || size == -1) {
            return false;
        }

        custom_istream in(std::make_unique<byte_array_streambuf>(cb, size));
        rebuild(in, os);

        return true;
    }

    static void rebuild(binary_istream& in, binary_ostream& out) {
        bitwise_lock lock { in };
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
            auto initial = in.read<5>();
            out.write<5>(initial);

            uintmax_t current = 0;
            while (current < entries) {
                auto bits = ilog(static_cast<int>(entries - current));
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

                auto codeword_length = in.read_bits(codeword_length_length);
                out.write<5>(codeword_length & 0b11111);
            }
        }

        // Lookup
        auto type = in.read<1>();
        out.write<4>(type);

        switch (type) {
            case 0: break; // no lookup
            case 1: {
                auto min = in.read<32>();
                auto max = in.read<32>();
                auto val_length = in.read<4>();
                auto sequence_flag = in.read<1>();

                out.write<32>(min)
                    .write<32>(max)
                    .write<4>(val_length)
                    .write<1>(sequence_flag);

                uint32_t quantvals = _book_maptype1_quantvals(entries, dimensions);
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

    protected:
    istream_ptr stream;

    private:
    std::vector<char> _m_data;
    std::vector<uint32_t> _m_offsets;
    uint32_t _m_codebook_count;
};

struct wwriff_chunk {
    std::streamoff offset;
    std::streamoff size;
};

enum known_chunks {
    FMT,
    CUE,
    LIST,
    SMPL,
    VORB,
    DATA,

    CHUNK_COUNT
};

static std::string chunk_map[CHUNK_COUNT] {
    "fmt ", "cue ", "LIST", "smpl", "vorb", "data"
};

wem_pcm_provider::wem_pcm_provider(const istream_ptr& stream) : pcm_provider(stream) {
    wave_header wave;
    stream->read(&wave, sizeof(wave));

    ASSERT(memcmp(wave.riff.header, "RIFF", 4) == 0);
    ASSERT(memcmp(wave.wave, "WAVE", 4) == 0);

    uint32_t riff_size = wave.riff.size + 8;

    std::streamoff offset = stream->tellg();

    wwriff_chunk chunks[CHUNK_COUNT] { };
    bool found[CHUNK_COUNT] { };
    riff_header chunk;

    while (offset < riff_size) {
        stream->seekg(offset);

        ASSERT((offset + 8) <= riff_size);

        stream->read(&chunk, sizeof(chunk));

        // Read all chunks
        for (size_t i = 0; i < CHUNK_COUNT; ++i) {
            if (memcmp(chunk.header, chunk_map[i].c_str(), 4) == 0) {
                chunks[i].offset = offset + 8;
                chunks[i].size = chunk.size;
                found[i] = true;
                break;
            }
        }

        offset = offset + sizeof(chunk) + chunk.size;
    }

    ASSERT(offset <= riff_size);

    ASSERT(found[FMT] && found[DATA]);

    for (int i = 0; i < CHUNK_COUNT; ++i) {
        utils::coutln(chunk_map[i] + ':', found[i] ? "found" : "missing");
    }

    utils::coutln("");

    wwriff_chunk& fmt = chunks[FMT];
    wwriff_chunk& data = chunks[DATA];
    wwriff_chunk& vorb = chunks[VORB];

    ASSERT(!found[VORB] && fmt.size == 66);

    ASSERT(!found[VORB] && !(fmt.size == 40 || fmt.size == 24 || fmt.size == 18));

    if (!found[VORB] && fmt.size == 66) {
        utils::coutln("faking vorb");
        vorb.offset = fmt.offset + 24;
    }

    // All chunks are read

    // Search back too far to read the format chunk entirely
    stream->seekg(fmt.offset - 8);

    fmt_chunk _fmt;
    stream->read(&_fmt, sizeof(_fmt));

    ASSERT(_fmt.format == 0xFFFF);
    ASSERT(_fmt.align == 0);
    ASSERT(_fmt.bits == 0);

    // Extra format length
    ASSERT(stream->read<uint16_t>() == fmt.size - 18);

    uint16_t ext_unk = 0;
    uint32_t subtype = 0;
    if ((fmt.size - 18) >= 2) {
        stream->read<uint16_t>(ext_unk);
        utils::coutln(fmt.size - 18, "extra bytes, read ext_unk:", ext_unk);
        if ((fmt.size - 18) >= 6) {
            stream->read<uint32_t>(subtype);
            utils::coutln("read subtype at", stream->tellg(), "value:", subtype);
        }
    }

    if (fmt.size == 40) {
        utils::coutln("confirming magic bytes at", stream->tellg());

        static constexpr uint8_t magic_buf[16] {
            1,   0,  0,   0,
            0,   0,  16,  0,
            128, 0,  0,   170,
            0,   56, 155, 113
        };

        uint8_t magic[16];
        stream->read(magic);

        ASSERT(memcmp(magic, magic_buf, sizeof(magic_buf)) == 0);
    }

    uint32_t cue_count = 0;
    // Read cue
    if (found[CUE]) {
        stream->seekg(chunks[CUE].offset);
        stream->read<uint32_t>(cue_count);
        utils::coutln("cue count:", cue_count);
    }

    // smpl
    uint32_t loop_count = 0;
    uint32_t loop_start = 0;
    uint32_t loop_end = 0;
    if (found[SMPL]) {
        stream->seekg(chunks[SMPL].offset + 28);
        stream->read<uint32_t>(loop_count);

        ASSERT(loop_count == 1);

        stream->seekg(chunks[SMPL].offset + 44);
        stream->read<uint32_t>(loop_start);
        stream->read<uint32_t>(loop_end);

        utils::coutln("loop count:", loop_count);
        utils::coutln("loop start:", loop_start);
        utils::coutln("loop_end:", loop_end);
    }

    // vorb
    switch (vorb.size) {
        case 0:  case 40: case 42:
        case 44: case 50: case 52:
            utils::coutln("valid vorb size", vorb.size);
            stream->seekg(vorb.offset);
            break;

        default: ASSERT(false);
    }

    uint32_t sample_count = stream->read<uint32_t>();
    utils::coutln(sample_count, "total samples");

    bool no_granule = false;
    bool mod_packets = false;
    switch (vorb.size) {
        case 0:
        case 42: {
            utils::coutln("no granule");
            no_granule = true;

            stream->seekg(vorb.offset + 4);
            uint32_t mod_signal = stream->read<uint32_t>();

            utils::coutln("mod signal:", mod_signal);

            if (mod_signal != 74 && mod_signal != 75 &&
                mod_signal != 105 && mod_signal != 112) {
                mod_packets = true;
            }

            utils::coutln(mod_packets ? "mod packets" : "default packets");

            stream->seekg(vorb.offset + 16);
            break;
        }

        default:
            stream->seekg(vorb.offset + 24);
    }

    uint32_t setup_offset = stream->read<uint32_t>();
    uint32_t audio_start_offset = stream->read<uint32_t>();

    utils::coutln("setup packet:", setup_offset);
    utils::coutln("audio offset:", audio_start_offset);

    switch (vorb.size) {
        case 0:
        case 42:
            utils::coutln("seeking 36 into vorb");
            stream->seekg(vorb.offset + 36);
            break;

        case 50:
        case 52:
            utils::coutln("seeking 44 into vorb");
            stream->seekg(vorb.offset + 44);
            break;

        default: break;
    }

    uint32_t uid = 0;
    uint8_t blocksize_0_pow = 0;
    uint8_t blocksize_1_pow = 0;

    switch (vorb.size) {
        case 40:
        case 44: ASSERT(false);

        case 0:
        case 42:
        case 50:
        case 52:
            utils::coutln("reading uid, blocksize0 and blocksize1");
            stream->read(uid);
            stream->read(blocksize_0_pow);
            stream->read(blocksize_1_pow);
            utils::coutln(uid, static_cast<int>(blocksize_0_pow), static_cast<int>(blocksize_1_pow));
            break;

        default: break;
    }

    if (loop_count) {
        if (loop_end == 0) {
            utils::coutln("loop end at end");
            loop_end = sample_count;
        } else {
            ++loop_end;
        }

        utils::coutln("loop from", loop_start, "to", loop_end);

        ASSERT(loop_start < sample_count && loop_end <= sample_count && loop_start <= loop_end);
    }

    switch (subtype) {
        case 4: utils::coutln("1 channel, no seek"); break;
        case 3: utils::coutln("2 channels"); break;
        case 51: utils::coutln("4 channels"); break;
        case 55: utils::coutln("5 channels, unspecified seek"); break;
        case 59: utils::coutln("5 channels, no seek"); break;
        case 63: utils::coutln("6 channels, no seek"); break;
        default: utils::coutln("unknown subtype", subtype); break;
    }

    utils::coutln("parsing done");

    binary_ostream os("C:\\Users\\Nuan\\Downloads\\wwriff.ogg");
    ogg_stream_state state;
    ogg_stream_init(&state, 1);
    ogg_page page;

    int64_t packetno = 0;

    vorbis_info vi;
    vorbis_info_init(&vi);

    vorbis_comment vc;
    vorbis_comment_init(&vc);

    {
        auto ss = std::make_shared<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary);
        binary_iostream io(ss);

        io.write("\x01vorbis", 7);

        {
            bitwise_lock<binary_ostream> lock { io };

            io.write<32>(0) // version
                .write<8>(_fmt.channels & 0xFF) // channels
                .write<32>(_fmt.rate) // sample rate
                .write<32>(0) // max bitrate
                .write<32>(_fmt.byte_rate * 8) // bitrate
                .write<32>(0) // min bitrate
                .write<4>(blocksize_0_pow) // blocksize0
                .write<4>(blocksize_1_pow) // blocksize1
                .write<1>(1); // framing
        }

        auto packet_data = ss->str();

        ogg_packet packet {
            .packet = reinterpret_cast<unsigned char*>(packet_data.data()),
            .bytes = static_cast<long>(packet_data.size()),
            .b_o_s = 1,
            .e_o_s = 0,
            .granulepos = 0,
            .packetno = packetno++
        };

        ogg_stream_packetin(&state, &packet);

        ASSERT(vorbis_synthesis_headerin(&vi, &vc, &packet) == 0);

        while (ogg_stream_flush(&state, &page)) {
            os.write(page.header, page.header_len);
            os.write(page.body, page.body_len);
            page = { };
        }
    }

    // Comment
    {
        auto ss = std::make_shared<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary);
        binary_iostream io(ss);

        io.write("\x03vorbis", 7);

        {
            bitwise_lock<binary_ostream> lock { io };

            static constexpr char vendor[] = "converted from Audiokinetic Wwise by ww2ogg 0.22";// "ww2ogg Nao implementation";

            io.write<32>(static_cast<uint32_t>(std::size(vendor) - 1));

            for (char c : vendor) {
                if (!c) {
                    break;
                }

                io.write<8>(c);
            }

            if (loop_count == 0) {
                // No comments
                io.write<32>(0);
            } else {
                // 2 comments
                io.write<32>(2);

                std::stringstream loop_ss;
                loop_ss << "LoopStart=" << loop_start;

                io.write<32>(static_cast<uint32_t>(loop_ss.str().size()));
                io.flush_bits();
                *ss << "LoopStart=" << loop_start;

                loop_ss.str("");
                loop_ss << "LoopEnd=" << loop_end;

                io.write<32>(static_cast<uint32_t>(loop_ss.str().size()));
                io.flush_bits();
                *ss << "LoopEnd=" << loop_end;
            }

            io.write<1>(1);
        }

        auto packet_data = ss->str();

        ogg_packet packet {
            .packet = reinterpret_cast<unsigned char*>(packet_data.data()),
            .bytes = static_cast<long>(packet_data.size()),
            .packetno = packetno++
        };
        ogg_stream_packetin(&state, &packet);

        ASSERT(vorbis_synthesis_headerin(&vi, &vc, &packet) == 0);

        while (ogg_stream_pageout(&state, &page)) {
            os.write(page.header, page.header_len);
            os.write(page.body, page.body_len);
            page = { };
        }
    }

    std::vector<bool> mode_blockflag;
    size_t mode_bits = 0;

    // Comment and setup can be on same page


    {
        auto ss = std::make_shared<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary);
        binary_iostream io(ss);

        io.write("\x05vorbis", 7);

        vorbis_packet setup_packet(stream, data.offset + setup_offset, no_granule);
        stream->seekg(setup_packet.this_offset());
        utils::coutln("start at", stream->tellg());
        {
            bitwise_lock lock { stream };
            bitwise_lock<binary_ostream> lock2 { io };
            auto codebook_count_less1 = stream->read<8>();

            uint32_t codebook_count = codebook_count_less1 + 1;
            utils::coutln("codebook count:", codebook_count);
            io.write<8>(codebook_count_less1);

            codebook_library cbl(std::make_shared<custom_istream>(IDR_PACKED_CODEBOOKS_AOTUV_603));

            for (uint32_t i = 0; i < codebook_count; ++i) {
                auto codebook_id = stream->read<10>();
                ASSERT(cbl.rebuild(codebook_id, io));
            }

            // Time domain transforms
            io.write<6>(0);
            io.write<16>(0);

            // Floors
            auto floor_count_less1 = stream->read<6>();
            uint32_t floor_count = floor_count_less1 + 1;
            io.write<6>(floor_count_less1);

            for (uint32_t i = 0; i < floor_count; ++i) {
                io.write<16>(1);

                auto floor1_partitions = stream->read<5>();
                io.write<5>(floor1_partitions);

                std::vector<uint8_t> floor1_partition_class_list(floor1_partitions);
                uint8_t max_class = 0;
                for (uint8_t j = 0; j < floor1_partitions; ++j) {
                    auto floor1_partition_class = stream->read<4>();
                    io.write<4>(floor1_partition_class);

                    floor1_partition_class_list[j] = floor1_partition_class;

                    if (floor1_partition_class > max_class) {
                        max_class = floor1_partition_class;
                    }
                }

                std::vector<uint8_t> floor1_class_dimension_list(max_class + 1ui64);
                for (uint8_t j = 0; j <= max_class; ++j) {
                    auto class_dimension_less1 = stream->read<3>();
                    io.write<3>(class_dimension_less1);

                    floor1_class_dimension_list[j] = class_dimension_less1 + 1;

                    auto subclasses = stream->read<2>();
                    io.write<2>(subclasses);

                    if (subclasses != 0) {
                        auto master_book = stream->read<8>();
                        io.write<8>(master_book);

                        ASSERT(master_book < codebook_count);
                    }

                    for (uint8_t k = 0; k < (1ui8 << subclasses); ++k) {
                        auto subclass_book_plus1 = stream->read<8>();
                        io.write<8>(subclass_book_plus1);

                        int16_t subclass_book = static_cast<int16_t>(subclass_book_plus1) - 1;
                        if (subclass_book >= 0 && static_cast<uint16_t>(subclass_book) >= codebook_count) {
                            ASSERT(false);
                        }
                        //ASSERT(!(subclass_book >= 0 && subclass_book >= codebook_count)); 
                    }
                }

                auto floor1_multiplier_less1 = stream->read<2>();
                io.write<2>(floor1_multiplier_less1);

                auto rangebits = stream->read<4>();
                io.write<4>(rangebits);

                for (uint8_t j = 0; j < floor1_partitions; ++j) {
                    uint8_t current_class = floor1_partition_class_list[j];
                    for (uint8_t k = 0; k < floor1_class_dimension_list[current_class]; ++k) {
                        auto val = stream->read_bits(rangebits);
                        io.write_bits(val, rangebits);
                    }
                }
            }

            // Residue
            auto residue_count_less1 = stream->read<6>();
            uint8_t residue_count = residue_count_less1 + 1;
            io.write<6>(residue_count_less1);

            for (uint8_t i = 0; i < residue_count; ++i) {
                auto type = stream->read<2>();
                io.write<16>(type);

                ASSERT(type <= 2);

                auto begin = stream->read<24>();
                auto end = stream->read<24>();
                auto residue_partition_size_less1 = stream->read<24>();
                auto residue_classifications_less1 = stream->read<6>();
                auto residue_classbook = stream->read<8>();

                uint8_t residue_classifications = residue_classifications_less1 + 1;

                io.write<24>(begin)
                    .write<24>(end)
                    .write<24>(residue_partition_size_less1)
                    .write<6>(residue_classifications_less1)
                    .write<8>(residue_classbook);

                ASSERT(residue_classbook < codebook_count);

                std::vector<uint16_t> cascade(residue_classifications);

                for (uint8_t j = 0; j < residue_classifications; ++j) {
                    auto low_bits = stream->read<3>();
                    io.write<3>(low_bits);

                    cascade[j] = low_bits;

                    auto flag = stream->read<1>();
                    io.write<1>(flag);

                    if (flag != 0) {
                        auto high_bits = stream->read<5>();
                        io.write<5>(high_bits);
                        cascade[j] |= (high_bits << 3);
                    }
                }

                for (uint8_t j = 0; j < residue_classifications; ++j) {
                    for (uint8_t k = 0; k < 8; ++k) {
                        if (cascade[j] & (1 << k)) {
                            auto book = stream->read<8>();
                            io.write<8>(book);

                            ASSERT(book < codebook_count);
                        }
                    }
                }
            }

            // Mapping
            auto mapping_count_less1 = stream->read<6>();
            uint8_t mapping_count = mapping_count_less1 + 1;
            io.write<6>(mapping_count_less1);

            for (uint8_t i = 0; i < mapping_count; ++i) {
                io.write<16>(0); // mapping type

                auto flag = stream->read<1>();
                io.write<1>(flag);

                uint8_t submaps = 1;
                if (flag != 0) {
                    auto submaps_less1 = stream->read<4>();
                    submaps = submaps_less1 + 1;
                    io.write<4>(submaps_less1);
                }

                auto square_polar_flag = stream->read<1>();
                io.write<1>(square_polar_flag);

                if (square_polar_flag != 0) {
                    auto coupling_steps_less1 = stream->read<8>();
                    uint16_t coupling_steps = coupling_steps_less1 + 1;
                    io.write<8>(coupling_steps_less1);

                    for (uint16_t j = 0; j < coupling_steps; ++j) {
                        const auto bits = ilog(_fmt.channels - 1);
                        auto magnitude = stream->read_bits(bits);
                        auto angle = stream->read_bits(bits);

                        io.write_bits(magnitude, bits).write_bits(angle, bits);
                        ASSERT(angle != magnitude && magnitude < _fmt.channels && angle < _fmt.channels);
                    }
                }

                auto reserved = stream->read<2>();
                io.write<2>(reserved);
                ASSERT(reserved == 0);

                if (submaps > 1) {
                    for (uint16_t j = 0; j < _fmt.channels; ++j) {
                        auto mux = stream->read<4>();
                        io.write<4>(mux);

                        ASSERT(mux < submaps);
                    }
                }

                for (uint16_t j = 0; j < submaps; ++j) {
                    auto config = stream->read<8>();
                    io.write<8>(config);

                    auto floor_number = stream->read<8>();
                    io.write<8>(floor_number);

                    ASSERT(floor_number < floor_count);

                    auto residue_number = stream->read<8>();
                    io.write<8>(residue_number);

                    ASSERT(residue_number < residue_count);
                }
            }

            // Mode
            auto mode_count_less1 = stream->read<6>();
            uint8_t mode_count = mode_count_less1 + 1;
            io.write<6>(mode_count_less1);

            mode_blockflag.resize(mode_count);
            mode_bits = ilog(mode_count - 1);

            for (uint8_t i = 0; i < mode_count; ++i) {
                auto flag = stream->read<1>();
                io.write<1>(flag);

                mode_blockflag[i] = flag != 0;

                io.write<16>(0) // window type
                    .write<16>(0); // transform type

                auto mapping = stream->read<8>();
                io.write<8>(mapping);

                ASSERT(mapping < mapping_count);
            }

            io.write<1>(1); // framing
            io.flush_bits();
        }

        auto packet_data = ss->str();

        ASSERT(setup_packet.next_offset() == (data.offset + audio_start_offset));

        ogg_packet packet {
            .packet = reinterpret_cast<unsigned char*>(packet_data.data()),
            .bytes = static_cast<long>(packet_data.size()),
            .packetno = packetno++
        };
        ogg_stream_packetin(&state, &packet);

        ASSERT(vorbis_synthesis_headerin(&vi, &vc, &packet) == 0);

        utils::coutln("setup packet of size", packet_data.size(), "starting at", os.tellp());

        while (ogg_stream_flush(&state, &page)) {
            os.write(page.header, page.header_len);
            os.write(page.body, page.body_len);
            page = { };
        }
    }

    utils::coutln("writing audio from", data.offset + audio_start_offset, "to", os.tellp());
    bool prev_blockflag = false;

    
    //ASSERT(vorbis_encode_setup_managed(&vi,
    //    _fmt.channels, _fmt.rate, 0, static_cast<long>(_fmt.byte_rate) * 8, 0) == 0);
    // Audio
    auto _offset = data.offset + audio_start_offset;
    int64_t last_bs = 0;
    int64_t granpos = 0;
    while (_offset < (data.offset + data.size)) {
        auto ss = std::make_shared<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary);
        binary_iostream io(ss);

        uint32_t granule;
        std::streamoff next_offset;
        {
            bitwise_lock<binary_ostream> lock { io };

            vorbis_packet audio_packet(stream, _offset, no_granule);

            granule = audio_packet.granule();
            next_offset = audio_packet.next_offset();

            ASSERT((_offset + audio_packet.header_size()) <= (data.offset + data.size));

            _offset = audio_packet.this_offset();
            stream->seekg(_offset);

            if (granule == 0xFFFFFFFF) {
                utils::coutln("fixing granule");
                granule = 1;
            }

            if (mod_packets) {
                ASSERT(!mode_blockflag.empty());

                io.write<1>(0); // type (audio)

                uintmax_t mode_number;
                uintmax_t remainder;
                {
                    bitwise_lock lock1 { stream };
                    mode_number = stream->read_bits(mode_bits);
                    io.write_bits(mode_number, mode_bits);

                    remainder = stream->read_bits(8 - mode_bits);
                }

                if (mode_blockflag[mode_number]) {
                    stream->seekg(next_offset);
                    bool next_blockflag = false;

                    if ((next_offset + audio_packet.header_size()) <= (data.offset + data.size)) {
                        vorbis_packet next_packet(stream, next_offset, no_granule);
                        auto next_size = next_packet.size();

                        if (next_size > 0) {
                            stream->seekg(next_packet.this_offset());

                            bitwise_lock lock1 { stream };

                            auto next_mode_number = stream->read_bits(mode_bits);
                            next_blockflag = mode_blockflag[next_mode_number];
                        }
                    }

                    io.write<1>(prev_blockflag ? 1 : 0)
                        .write<1>(next_blockflag ? 1 : 0);

                    stream->seekg(_offset + 1);
                }

                prev_blockflag = mode_blockflag[mode_number];
                io.write_bits(remainder, 8 - mode_bits);
            } else {
                io.write<8>(stream->read<uint8_t>());
            }

            // Remainder of packet
            for (int64_t i = 1; i < audio_packet.size(); ++i) {
                io.write<8>(stream->read<uint8_t>());
            }

        }

        _offset = next_offset;

        auto packet_data = ss->str();

        ogg_packet packet {
            .packet = reinterpret_cast<unsigned char*>(packet_data.data()),
            .bytes = static_cast<long>(packet_data.size()),
            .granulepos = granule,
            .packetno = packetno++
        };

        int64_t bs = vorbis_packet_blocksize(&vi, &packet);
        ASSERT(bs > 0);
        if (last_bs) {
            granpos += (last_bs + bs) / 4;
        }

        last_bs = bs;

        packet.granulepos = granpos;

        if (_offset >= (data.offset + data.size)) {
            packet.e_o_s = 1;
        }
        ogg_stream_packetin(&state, &packet);

        while (ogg_stream_pageout(&state, &page)) {
            os.write(page.header, page.header_len);
            os.write(page.body, page.body_len);
        }
    }

    vorbis_info_clear(&vi);
    vorbis_comment_clear(&vc);

    while (ogg_stream_flush(&state, &page) != 0) {
        os.write(page.header, page.header_len);
        os.write(page.body, page.body_len);
    }

    ogg_stream_clear(&state);
}

wem_pcm_provider::~wem_pcm_provider() {
    
}

int64_t wem_pcm_provider::get_samples(void*& data, sample_type type) {
    return PCM_ERR;
}

int64_t wem_pcm_provider::rate() const {
    return 0;
}

int64_t wem_pcm_provider::channels() const {
    return 0;
}

std::chrono::nanoseconds wem_pcm_provider::duration() const {
    return std::chrono::nanoseconds(0);
}

std::chrono::nanoseconds wem_pcm_provider::pos() const {
    return std::chrono::nanoseconds(0);
}

void wem_pcm_provider::seek(std::chrono::nanoseconds pos) {
    
}

sample_type wem_pcm_provider::types() const {
    return SAMPLE_NONE;
}

sample_type wem_pcm_provider::preferred_type() const {
    return SAMPLE_NONE;
}
