#pragma once

#include "pcm_provider.h"

class flac_decoder;

class flac_pcm_provider : public pcm_provider {
    public:
    explicit flac_pcm_provider(const istream_ptr& stream);
    ~flac_pcm_provider();

    int64_t get_samples(void*& data, sample_type type) override;
    int64_t rate() const override;
    int64_t channels() const override;

    std::chrono::nanoseconds duration() const override;
    std::chrono::nanoseconds pos() const override;
    void seek(std::chrono::nanoseconds pos) override;

    sample_type types() const override;
    sample_type preferred_type() const override;

    private:
    std::unique_ptr<flac_decoder> _m_decoder;

    std::chrono::nanoseconds _m_ns_per_frame;
};