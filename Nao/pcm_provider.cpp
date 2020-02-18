#include "pcm_provider.h"

pcm_provider::pcm_provider(istream_ptr stream) : stream(std::move(stream)) {
    
}

pcm_provider::~pcm_provider() {
    
}

size_t pcm_provider::sample_size(sample_type type) {
    switch (type) {
        case SAMPLE_INT16:
            return sizeof(int16_t);
        case SAMPLE_FLOAT32:
            return sizeof(float);
        default: return 0;
    }
}


PaSampleFormat pcm_provider::pa_format(sample_type type) {
    switch (type) {
        case SAMPLE_INT16: return paInt16;
        case SAMPLE_FLOAT32: return paFloat32;
        default: return 0;
    }
}

void pcm_provider::delete_samples(void* data, sample_type type) {
    switch (type) {
        case SAMPLE_INT16:
            delete[] static_cast<underlying_type_t<SAMPLE_INT16>*>(data);
            break;

        case SAMPLE_FLOAT32:
            delete[] static_cast<underlying_type_t<SAMPLE_FLOAT32>*>(data);
            break;

        default: break;
    }
}

void pcm_provider::convert_samples(void* from, sample_type src_type,
        void*& to, sample_type dest_type, int64_t frames, int64_t channels) {
    switch (src_type) {
        case SAMPLE_INT16: {
            switch (dest_type) {
                case SAMPLE_FLOAT32: {
                    // ReSharper disable once CppNonReclaimedResourceAcquisition
                    auto conv = new float[frames * channels];
                    to = conv;
                    auto src = static_cast<int16_t*>(from);

                    for (int64_t i = 0; i < (frames * channels); ++i) {
                        conv[i] = static_cast<float>(src[i]) / static_cast<float>(1 << ((sizeof(int16_t) * CHAR_BIT) - 1));
                    }
                    break;
                }

                case SAMPLE_INT16:
                case SAMPLE_NONE: break;
            }
            break;
        }

        case SAMPLE_FLOAT32: {
            switch (dest_type) {
                case SAMPLE_INT16: {
                    // ReSharper disable once CppNonReclaimedResourceAcquisition
                    auto conv = new int16_t[frames * channels];
                    to = conv;
                    auto src = static_cast<float*>(from);

                    for (int64_t i = 0; i < (frames * channels); ++i) {
                        conv[i] = static_cast<int16_t>(
                            std::clamp<float>(src[i] * (1 << ((sizeof(int16_t) * CHAR_BIT) - 1)),
                                std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max()));

                    }
                    break;
                }

                case SAMPLE_FLOAT32:
                case SAMPLE_NONE: break;
            }
            break;
        }

        case SAMPLE_NONE: break;
    }
}
