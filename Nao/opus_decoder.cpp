#include "opus_decoder.h"

#include "utils.h"
#include "namespaces.h"

OpusFileCallbacks opus_decoder::callbacks {
    .read = [](void* source, unsigned char* data, int count) -> int {
        binary_istream& stream = *static_cast<binary_istream*>(source);

        if (!stream.good() || stream.eof()) {
            return 0;
        }

        return static_cast<int>(stream.read(data, count).gcount());
    },

    .seek = [](void* source, opus_int64 offset, int whence) {
        binary_istream& stream = *static_cast<binary_istream*>(source);

        switch (whence) {
            case SEEK_SET: stream.seekg(offset, std::ios::beg); break;
            case SEEK_CUR: stream.seekg(offset, std::ios::cur); break;
            case SEEK_END: stream.seekg(offset, std::ios::end); break;
            default: return -1;
        }

        return 0;
    },

    .tell = [](void* source) -> opus_int64 {
        return static_cast<binary_istream*>(source)->tellg();
    },

    .close = [](void*) -> int { return 0; }
};

opus_decoder::opus_decoder(const istream_ptr& stream)
    : opus_vorbis_decoder(stream)
    , _m_of(op_test_callbacks(stream.get(), &callbacks,
        nullptr, 0, nullptr), op_free) {

    if (_m_of != nullptr) {
        ASSERT(op_test_open(_m_of.get()) == 0);
        _m_head = op_head(_m_of.get(), -1);

        _m_ns_per_sample = std::chrono::duration_cast<std::chrono::nanoseconds>(
            1s / static_cast<double>(_m_head->input_sample_rate));

        is_valid = true;

        total_duration = op_pcm_total(_m_of.get(), -1) * _m_ns_per_sample;
        sample_rate = _m_head->input_sample_rate;
        channel_count = _m_head->channel_count;
        
    }
}

std::chrono::nanoseconds opus_decoder::pos() const {
    return op_pcm_tell(_m_of.get()) * _m_ns_per_sample;
}

void opus_decoder::seek(std::chrono::nanoseconds pos) const {
    ASSERT(op_pcm_seek(_m_of.get(), pos / _m_ns_per_sample) == 0);
}


int64_t opus_decoder::read(short* buffer, int64_t frames) const {
    return op_read(_m_of.get(), buffer, static_cast<int>(frames / channel_count), nullptr);
}

int64_t opus_decoder::read_float(float* buffer, int64_t frames) const {
    return op_read_float(_m_of.get(), buffer, static_cast<int>(frames / channel_count), nullptr);
}


