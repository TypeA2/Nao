#include "vorbis_decoder.h"

#include "utils.h"
#include "namespaces.h"

ov_callbacks vorbis_decoder::callbacks {
    .read_func = [](void* ptr, size_t size, size_t nmemb, void* source) -> size_t {
        binary_istream& stream = *static_cast<binary_istream*>(source);

        if (!stream.good() || stream.eof()) {
            return 0;
        }

        return stream.read(ptr, size, nmemb).gcount();
    },

    .seek_func = [](void* source, ogg_int64_t offset, int whence) -> int {
        binary_istream& stream = *static_cast<binary_istream*>(source);

        switch (whence) {
            case SEEK_SET: stream.seekg(offset, std::ios::beg); break;
            case SEEK_CUR: stream.seekg(offset, std::ios::cur); break;
            case SEEK_END: stream.seekg(offset, std::ios::end); break;
            default: return -1;
        }

        return 0;
    },

    .tell_func = [](void* source) -> long {
        return static_cast<long>(static_cast<binary_istream*>(source)->tellg());
    }
};

vorbis_decoder::vorbis_decoder(const istream_ptr& stream)
    : opus_vorbis_decoder(stream)
    , _m_vf { new OggVorbis_File, ov_clear } {
    if (ov_test_callbacks(stream.get(), _m_vf.get(), nullptr, 0, callbacks) == 0) {
        ASSERT(ov_test_open(_m_vf.get()) == 0);
        _m_info = ov_info(_m_vf.get(), -1);

        is_valid = true;

        total_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(ov_time_total(_m_vf.get(), -1) * 1s);
        sample_rate = _m_info->rate;
        channel_count = _m_info->channels;
    }
}

std::chrono::nanoseconds vorbis_decoder::pos() const {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(ov_time_tell(_m_vf.get()) * 1s);
}

void vorbis_decoder::seek(std::chrono::nanoseconds pos) const {
    ov_time_seek(_m_vf.get(), static_cast<double>(pos.count()) / std::chrono::nanoseconds::period::den);
}

int64_t vorbis_decoder::read(short* buffer, int64_t frames) const {
    int bs;
    long ret = ov_read(_m_vf.get(), reinterpret_cast<char*>(buffer),
        static_cast<int>(frames * channel_count * sizeof(short)), 0, 2, 1, &bs);

    if (ret <= 0) {
        return ret;
    }

    return ret / sizeof(short) / channel_count;
}

int64_t vorbis_decoder::read_float(float* buffer, int64_t frames) const {
    int bs;
    float** pcm;
    long frames_read = ov_read_float(_m_vf.get(), &pcm, static_cast<int>(frames * channel_count), &bs);
    if (frames_read <= 0) {
        return frames_read;
    }

    // Interleave
    for (long i = 0; i < frames_read; ++i) {
        for (int j = 0; j < channel_count; ++j) {
            *buffer++ = pcm[j][i];
        }
    }

    return frames_read;
}
