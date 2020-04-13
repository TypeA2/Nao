#pragma once

#include "pcm_provider.h"

#include "ffmpeg.h"

class ffmpeg_pcm_provider : public pcm_provider {
    int64_t _samples_played = 0;

    ffmpeg::avformat::context _ctx;
    ffmpeg::avformat::stream _stream;
    ffmpeg::avcodec::codec _codec;
    ffmpeg::avcodec::context _codec_ctx;

    ffmpeg::frame _frame;
    ffmpeg::packet _packet;

    sample_format _fmt;
    uint64_t _channel_layout;
    uint8_t _channels;

    public:
    explicit ffmpeg_pcm_provider(const istream_ptr& stream, const std::string& path = "");
    ~ffmpeg_pcm_provider() override = default;

    pcm_samples get_samples() override;
    int64_t rate() override;
    int64_t channels() override;
    std::string name() override;

    std::chrono::nanoseconds duration() override;
    std::chrono::nanoseconds pos() override;
    void seek(std::chrono::nanoseconds pos) override;

    sample_format format() override;
};
