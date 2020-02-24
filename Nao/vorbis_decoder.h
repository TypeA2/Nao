#pragma once

#include "opus_vorbis_decoder.h"

#include <vorbis/vorbisfile.h>

// Light wrapper over libvorbisfile
class vorbis_decoder : public opus_vorbis_decoder {
    public:
    static ov_callbacks callbacks;

    explicit vorbis_decoder(const istream_ptr& stream);
    ~vorbis_decoder() = default;

    std::chrono::nanoseconds pos() const override;
    void seek(std::chrono::nanoseconds pos) const override;

    [[nodiscard]] int64_t read(short* buffer, int64_t frames) const override;
    [[nodiscard]] int64_t read_float(float* buffer, int64_t frames) const override;

    private:
    std::shared_ptr<OggVorbis_File> _m_vf;
    vorbis_info* _m_info { };
};
