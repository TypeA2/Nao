#include "pcm_provider.h"
#include "utils.h"

namespace samples {
    size_t sample_size(sample_format fmt) {
        switch (fmt & sample_format::type_mask) {
            case sample_format::none:    return 0;
            case sample_format::uint8:   return 1;
            case sample_format::int16:   return 2;
            case sample_format::int32:   return 4;
            case sample_format::int64:   return 8;
            case sample_format::float32: return 4;
            case sample_format::float64: return 8;
            default:                     return 0;
        }
    }

    AVSampleFormat to_avutil(sample_format fmt) {
        switch (fmt) {
            case sample_format::uint8:    return AV_SAMPLE_FMT_U8;
            case sample_format::int16:    return AV_SAMPLE_FMT_S16;
            case sample_format::int32:    return AV_SAMPLE_FMT_S32;
            case sample_format::int64:    return AV_SAMPLE_FMT_S64;
            case sample_format::float32:  return AV_SAMPLE_FMT_FLT;
            case sample_format::float64:  return AV_SAMPLE_FMT_DBL;
            case sample_format::uint8p:   return AV_SAMPLE_FMT_U8P;
            case sample_format::int16p:   return AV_SAMPLE_FMT_S16P;
            case sample_format::int32p:   return AV_SAMPLE_FMT_S32P;
            case sample_format::int64p:   return AV_SAMPLE_FMT_S64P;
            case sample_format::float32p: return AV_SAMPLE_FMT_FLTP;
            case sample_format::float64p: return AV_SAMPLE_FMT_DBLP;
            default:                      return AV_SAMPLE_FMT_NONE;
        }
    }

    sample_format from_avutil(AVSampleFormat fmt) {
        switch (fmt) {
            case AV_SAMPLE_FMT_U8:   return sample_format::uint8;
            case AV_SAMPLE_FMT_S16:  return sample_format::int16;
            case AV_SAMPLE_FMT_S32:  return sample_format::int32;
            case AV_SAMPLE_FMT_S64:  return sample_format::int64;
            case AV_SAMPLE_FMT_FLT:  return sample_format::float32;
            case AV_SAMPLE_FMT_DBL:  return sample_format::float64;
            case AV_SAMPLE_FMT_U8P:  return sample_format::uint8p;
            case AV_SAMPLE_FMT_S16P: return sample_format::int16p;
            case AV_SAMPLE_FMT_S32P: return sample_format::int32p;
            case AV_SAMPLE_FMT_S64P: return sample_format::int64p;
            case AV_SAMPLE_FMT_FLTP: return sample_format::float32p;
            case AV_SAMPLE_FMT_DBLP: return sample_format::float64p;
            default:                 return sample_format::none;
        }
    }

    bool is_planar(sample_format fmt) {
        return static_cast<std::underlying_type_t<sample_format>>(fmt & sample_format::planar_flag);
    }

    std::string format_name(sample_format fmt) {
        switch (fmt) {
            case sample_format::uint8:    return "uint8";
            case sample_format::int16:    return "int16";
            case sample_format::int32:    return "int32";
            case sample_format::int64:    return "int64";
            case sample_format::float32:  return "float";
            case sample_format::float64:  return "double";
            case sample_format::uint8p:   return "uint8 planar";
            case sample_format::int16p:   return "int16 planar";
            case sample_format::int32p:   return "int32 planar";
            case sample_format::int64p:   return "int64 planar";
            case sample_format::float32p: return "float planar";
            case sample_format::float64p: return "double planar";
            default:                      return "none";
        }
    }


}

pcm_samples::pcm_samples(sample_format type, uint64_t frames, uint8_t channels, uint64_t channel_layout)
    : _type { type }, _frames { frames }, _channels { channels }, _channel_layout { channel_layout }
    , _data(_frames * _channels * samples::sample_size(type), 0) {
    
}

int64_t pcm_samples::frames() const {
    return _frames;
}

uint8_t pcm_samples::channels() const {
    return _channels;
}

int64_t pcm_samples::samples() const {
    return _frames * channels();
}

size_t pcm_samples::bytes() const {
    return _data.size();
}

char* pcm_samples::data() {
    return _data.data();
}

const char* pcm_samples::data() const {
    return _data.data();
}

pcm_samples::operator bool() const {
    return _frames > 0 && _channels > 0 && !_data.empty() && _type != sample_format::none;
}

pcm_provider::pcm_provider(const istream_ptr& stream) : stream(stream) {
    
}
