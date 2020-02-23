#pragma once

#include "pcm_provider.h"

class ogg_pcm_provider_impl;
class ogg_pcm_provider : public pcm_provider {
    public:
    explicit ogg_pcm_provider(const istream_ptr& stream);
    ~ogg_pcm_provider() override;

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
    void _validate_open() const;

    std::unique_ptr<ogg_pcm_provider_impl> _d;
};
