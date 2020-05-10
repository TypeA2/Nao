#pragma once

#include "image_provider.h"

#include "ffmpeg.h"

class ffmpeg_image_provider : public image_provider {
    ffmpeg::avformat::context _ctx;

    AVPixelFormat _fmt;
    dimensions _dims;

    image_data _data;

    public:
    explicit ffmpeg_image_provider(istream_ptr s, const std::string& path);

    ~ffmpeg_image_provider() override = default;

    image_data data() override;
    dimensions dims() override;
    AVPixelFormat format() override;
};
