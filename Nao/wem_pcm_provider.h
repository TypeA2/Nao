#pragma once

#include "pcm_provider.h"

#include <vorbis/vorbisfile.h>

class wem_pcm_provider : public pcm_provider {
    public:
    explicit wem_pcm_provider(const istream_ptr& stream);
    ~wem_pcm_provider() override = default;

    pcm_samples get_samples(sample_type type) override;
    int64_t rate() override;
    int64_t channels() override;
    channel_order order() override;
    std::string name() override;

    std::chrono::nanoseconds duration() override;
    std::chrono::nanoseconds pos() override;
    void seek(std::chrono::nanoseconds pos) override;

    sample_type types() override;
    sample_type preferred_type() override;

    private:
    void _validate_open();

    iostream_ptr _m_buf;
    OggVorbis_File _m_vf;
    vorbis_info* _m_info;
    bool _m_eof;
    std::chrono::nanoseconds _m_duration;
};
