#pragma once

#include "pcm_provider.h"

class flac_decoder;

class flac_pcm_provider : public pcm_provider {
    public:
    explicit flac_pcm_provider(const istream_ptr& stream);
    ~flac_pcm_provider() override = default;

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
    std::unique_ptr<flac_decoder> _m_decoder;

    std::chrono::nanoseconds _m_ns_per_frame;
};