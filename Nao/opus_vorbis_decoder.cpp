#include "opus_vorbis_decoder.h"

opus_vorbis_decoder::opus_vorbis_decoder(const istream_ptr& stream) : stream { stream } {
    
}

bool opus_vorbis_decoder::valid() const {
    return is_valid;
}

std::chrono::nanoseconds opus_vorbis_decoder::duration() const {
    return total_duration;
}

uint32_t opus_vorbis_decoder::rate() const {
    return sample_rate;
}

int opus_vorbis_decoder::channels() const {
    return channel_count;
}
