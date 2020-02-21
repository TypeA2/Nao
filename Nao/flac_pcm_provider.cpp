#include "flac_pcm_provider.h"

#include "utils.h"

#include <FLAC++/decoder.h>

class flac_decoder : public FLAC::Decoder::Stream {
    public:
    explicit flac_decoder(const istream_ptr& stream) : _m_stream { stream }, _m_info { }, _m_ready { }, _m_current_header { } { }

    FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t* bytes) override;
    FLAC__StreamDecoderSeekStatus seek_callback(FLAC__uint64 absolute_byte_offset) override;
    FLAC__StreamDecoderTellStatus tell_callback(FLAC__uint64* absolute_byte_offset) override;
    FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64* stream_length) override;
    bool eof_callback() override;

    FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame* frame, const FLAC__int32* const buffer[]) override;
    void metadata_callback(const FLAC__StreamMetadata* metadata) override;
    void error_callback(FLAC__StreamDecoderErrorStatus status) override;

    uint32_t get_sample_rate() const override { return _m_info.sample_rate; }
    uint32_t get_channels() const override { return _m_info.channels; }
    uint32_t get_bits_per_sample() const override { return _m_info.bits_per_sample; }
    FLAC__uint64 get_total_samples() const override { return _m_info.total_samples; }

    bool ready() const { return _m_ready; }

    const std::vector<std::vector<FLAC__int32>>& release_samples() {
        if (_m_ready) {
            _m_ready = false;
        }

        return _m_samples;
    }

    const FLAC__FrameHeader& current_header() const { return _m_current_header; }

    private:
    istream_ptr _m_stream;
    std::vector<std::vector<FLAC__int32>> _m_samples;
    FLAC__StreamMetadata_StreamInfo _m_info;
    bool _m_ready;
    FLAC__FrameHeader _m_current_header;
};

flac_pcm_provider::flac_pcm_provider(const istream_ptr& stream) : pcm_provider(stream)
    , _m_decoder { std::make_unique<flac_decoder>(stream) } {

    ASSERT(_m_decoder->set_md5_checking(true));
    ASSERT(_m_decoder->init() == FLAC__STREAM_DECODER_INIT_STATUS_OK);

    ASSERT(_m_decoder->process_until_end_of_metadata());
    ASSERT(_m_decoder->get_bits_per_sample() == 16);

    _m_ns_per_frame = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::seconds(1) / static_cast<double>(_m_decoder->get_sample_rate()));
}

int64_t flac_pcm_provider::get_samples(void*& data, sample_type type) {
    if (type != preferred_type()) {
        return PCM_ERR;
    }

    // Make sure there is audio available
    while (!_m_decoder->ready()) {
        if (_m_decoder->get_state() == FLAC__STREAM_DECODER_END_OF_STREAM) {
            return PCM_DONE;
        }

        _m_decoder->process_single();
    }

    const auto& samples = _m_decoder->release_samples();

    switch (preferred_type()) {
        case SAMPLE_INT16: {
            // ReSharper disable once CppNonReclaimedResourceAcquisition
            data = new int16_t[static_cast<int64_t>(_m_decoder->get_blocksize()) * _m_decoder->get_channels()];

            // Interleave
            int16_t* cur = static_cast<int16_t*>(data);
            for (int64_t i = 0; i < _m_decoder->get_blocksize(); ++i) {
                for (const auto& chan : samples) {
                    *cur++ = chan[i];

                }
            }
            break;
        }

        case SAMPLE_FLOAT32:
        case SAMPLE_NONE: return PCM_ERR;
    }

    return _m_decoder->get_blocksize();
}

int64_t flac_pcm_provider::rate() const {
    return _m_decoder->get_sample_rate();
}

int64_t flac_pcm_provider::channels() const {
    return _m_decoder->get_channels();
}

std::chrono::nanoseconds flac_pcm_provider::duration() const {
    return _m_decoder->get_total_samples() * _m_ns_per_frame;
}

std::chrono::nanoseconds flac_pcm_provider::pos() const {
    return _m_decoder->current_header().number.sample_number * _m_ns_per_frame;
}

void flac_pcm_provider::seek(std::chrono::nanoseconds pos) {
    if (_m_decoder->get_state() == FLAC__STREAM_DECODER_END_OF_STREAM) {
        _m_decoder->reset();
        _m_decoder->set_md5_checking(true);
    }

    _m_decoder->seek_absolute(pos / _m_ns_per_frame);
}

sample_type flac_pcm_provider::types() const {
    return preferred_type();
}

sample_type flac_pcm_provider::preferred_type() const {
    switch (_m_decoder->get_bits_per_sample()) {
        case 16: return SAMPLE_INT16;
        default: return SAMPLE_NONE;
    }
}

FLAC__StreamDecoderReadStatus flac_decoder::read_callback(FLAC__byte buffer[], size_t* bytes) {
    if (!_m_stream->good() || _m_stream->eof()) {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    _m_stream->read(buffer, *bytes);

    *bytes = _m_stream->gcount();

    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderSeekStatus flac_decoder::seek_callback(FLAC__uint64 absolute_byte_offset) {
    _m_stream->seekg(absolute_byte_offset);

    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus flac_decoder::tell_callback(FLAC__uint64* absolute_byte_offset) {
    if (!_m_stream->good()) {
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
    }

    *absolute_byte_offset = _m_stream->tellg();

    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus flac_decoder::length_callback(FLAC__uint64* stream_length) {
    if (!_m_stream->good()) {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    }

    auto cur = _m_stream->tellg();
    _m_stream->seekg(0, std::ios::end);
    *stream_length = _m_stream->tellg();
    _m_stream->seekg(cur, std::ios::beg);

    return _m_stream->good() ? FLAC__STREAM_DECODER_LENGTH_STATUS_OK : FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
}

bool flac_decoder::eof_callback() {
    return _m_stream->eof();
}

FLAC__StreamDecoderWriteStatus flac_decoder::write_callback(const FLAC__Frame* frame, const FLAC__int32* const buffer[]) {
    _m_current_header = frame->header;

    // Copy all data
    size_t i = 0;
    for (auto& channel : _m_samples) {
        std::copy(buffer[i], buffer[i] + frame->header.blocksize, channel.data());

        ++i;
    }

    _m_ready = true;

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void flac_decoder::metadata_callback(const FLAC__StreamMetadata* metadata) {
    switch (metadata->type) {
        case FLAC__METADATA_TYPE_STREAMINFO: {
            _m_info = metadata->data.stream_info;
            _m_samples.resize(_m_info.channels);
            for (auto& chan : _m_samples) {
                chan.resize(_m_info.max_blocksize);
            }
            break;
        }

        default: break;
    }
}

void flac_decoder::error_callback(FLAC__StreamDecoderErrorStatus status) {
    utils::coutln("FLAC error:", status);
}
