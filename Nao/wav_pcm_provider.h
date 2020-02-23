#pragma once

#include "pcm_provider.h"

#include "riff.h"

class wav_pcm_provider : public pcm_provider {
    public:
    explicit wav_pcm_provider(const istream_ptr& stream);
    ~wav_pcm_provider() override = default;

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
    static constexpr size_t frames_per_block = 1024;

    wave_header _m_wave;
    fmt_chunk _m_fmt;
    riff_header _m_data;

    std::streampos _m_data_start;
    std::chrono::nanoseconds _m_ns_per_frame;
};