#pragma once

#include "pcm_provider.h"

class vorbis_decoder;

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
    iostream_ptr _m_buf;
    std::unique_ptr<vorbis_decoder> _m_dec;
};
