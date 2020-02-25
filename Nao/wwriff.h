#pragma once

#include "binary_stream.h"

namespace wwriff {
    // Taken from libvorbis
    int ilog(unsigned int v);
    long book_maptype1_quantvals(long entries, long dim);

    // Convert a Wwise RIFF file to a valid ogg file
    bool wwriff_to_ogg(const istream_ptr& in, const ostream_ptr& out);
}

class vorbis_packet {
    public:
    explicit vorbis_packet(const istream_ptr& stream, std::streamoff offset, bool no_granule);
    vorbis_packet() = default;

    // Granule means the header is larger
    std::streamsize header_size() const;
    std::streamoff this_offset() const;
    std::streamoff next_offset() const;
    std::streamsize size() const;
    uint32_t granule() const;

    private:
    std::streamoff _m_offset { };
    std::streamsize _m_size { };
    uint32_t _m_granule { };
    bool _m_no_granule { };
};


class codebook_library {
    public:
    explicit codebook_library(const istream_ptr& codebook);

    const char* get_codebook(uint32_t id) const;
    std::streamsize get_size(uint32_t id) const;

    bool rebuild(uint32_t id, binary_ostream& os) const;

    static void rebuild(binary_istream& in, binary_ostream& out);

    protected:
    istream_ptr stream;

    private:
    std::vector<char> _m_data;
    std::vector<uint32_t> _m_offsets;
    uint32_t _m_codebook_count;
};

class ogg_stream;
class vorbis_encoder;

class wwriff_converter {
    struct wwriff_chunk {
        std::streamoff offset;
        std::streamoff size;
    };

    enum known_chunks {
        FMT, CUE, LIST, SMPL, VORB, DATA,
        CHUNK_COUNT
    };

    uint32_t _riff_size { };
    bool _chunks_found[CHUNK_COUNT] { };
    wwriff_chunk _chunks[CHUNK_COUNT] { };

    uint32_t _cue_count { };
    uint32_t _loop_count { };
    uint32_t _loop_start { };
    uint32_t _loop_end { };

    uint32_t _setup_offset { };
    uint32_t _audio_offset { };

    uint32_t _channels { };
    uint32_t _rate { };
    uint32_t _bitrate { };

    uint8_t _blocksize_0_pow { };
    uint8_t _blocksize_1_pow { };

    uint32_t _codebook_count { };
    uint32_t _floor_count { };
    uint32_t _residue_count { };
    uint32_t _mapping_count { };

    bool _mod_packets { };
    std::vector<bool> _mode_flag;
    size_t _mode_bits { };

    bool _parsed { };

    public:
    wwriff_converter(const istream_ptr& in);

    bool parse();
    bool convert(const ostream_ptr& out);

    private:
    bool _validate_header();
    bool _gather_chunks();
    bool _validate_chunks();
    bool _parse_chunks();
    bool _parse_fmt();
    bool _parse_cue();
    bool _parse_smpl();
    bool _parse_vorb();

    bool _write_header(ogg_stream& os, vorbis_encoder& vc) const;
    bool _write_comment(ogg_stream& os, vorbis_encoder& vc) const;
    bool _write_setup(ogg_stream& os, vorbis_encoder& vc);
    bool _write_audio(ogg_stream& os, vorbis_encoder& vc) const;

    bool _write_floors(binary_ostream& out);
    bool _write_residue(binary_ostream& out);
    bool _write_mapping(binary_ostream& out);
    bool _write_mode(binary_ostream& out);

    protected:
    istream_ptr in;

    private:
    static std::string chunk_map[CHUNK_COUNT];
};