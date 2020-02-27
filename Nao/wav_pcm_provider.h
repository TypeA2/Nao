#pragma once

#include "pcm_provider.h"

#include "riff.h"

class wav_pcm_provider : public pcm_provider {
    static constexpr size_t frames_per_block = 1024;

    fmt_chunk _fmt {};
    fmt_chunk_extensible _fmt_ex {};
    riff_header _data {};

    std::streampos _data_start;
    std::chrono::nanoseconds _ns_per_frame {};

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
};