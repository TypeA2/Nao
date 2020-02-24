#pragma once

#include "opus_vorbis_decoder.h"

#include <opusfile.h>

// Light wrapper over libvorbisfile
class opus_decoder : public opus_vorbis_decoder {
    public:
    static OpusFileCallbacks callbacks;

    explicit opus_decoder(const istream_ptr& stream);
    ~opus_decoder() = default;

    std::chrono::nanoseconds pos() const override;
    void seek(std::chrono::nanoseconds pos) const override;

    [[nodiscard]] int64_t read(short* buffer, int64_t frames) const override;
    [[nodiscard]] int64_t read_float(float* buffer, int64_t frames) const override;

    private:
    std::shared_ptr<OggOpusFile> _m_of;
    OpusHead const* _m_head { };
    std::chrono::nanoseconds _m_ns_per_sample { };
};
