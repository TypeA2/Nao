#pragma once

#include "pcm_provider.h"

class opus_vorbis_decoder;

enum ogg_pcm_type {
    TYPE_VORBIS,
    TYPE_OPUS
};

class ogg_pcm_provider : public pcm_provider {
    public:
    explicit ogg_pcm_provider(const istream_ptr& stream);
    ~ogg_pcm_provider() override = default;

    pcm_samples get_samples(sample_format type) override;
    int64_t rate() override;
    int64_t channels() override;
    channel_order order() override;
    std::string name() override;

    std::chrono::nanoseconds duration() override;
    std::chrono::nanoseconds pos() override;
    void seek(std::chrono::nanoseconds pos) override;

    sample_format types() override;
    sample_format preferred_type() override;

    private:
    std::unique_ptr<opus_vorbis_decoder> _m_dec;
    ogg_pcm_type _m_type;
};
