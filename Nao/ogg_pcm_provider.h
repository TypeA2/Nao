#pragma once

#include "pcm_provider.h"

class ogg_pcm_provider_impl;
class ogg_pcm_provider : public pcm_provider {
    public:
    explicit ogg_pcm_provider(istream_ptr stream);
    ~ogg_pcm_provider();

    int64_t get_samples(void*& data, sample_type type) override;
    int64_t rate() const override;
    int64_t channels() const override;

    std::chrono::nanoseconds duration() const override;
    std::chrono::nanoseconds pos() const override;
    void seek(std::chrono::nanoseconds pos) override;

    sample_type types() const override;
    sample_type preferred_type() const override;

    private:
    void _validate_open() const;

    std::unique_ptr<ogg_pcm_provider_impl> _d;
};

sample_type operator|(sample_type left, sample_type right);
