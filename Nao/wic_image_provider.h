#pragma once

#include "image_provider.h"

#include "wic.h"

class wic_image_provider : public image_provider {
    wic::imaging_factory _factory;
    wic::format_converter _converter;

    wic::bitmap_decoder _decoder;
    wic::bitmap_frame_decode _frame;

    dimensions _dims;

    public:
    static bool supports(istream_ptr stream);

    explicit wic_image_provider(istream_ptr stream);
    ~wic_image_provider() override = default;

    image_data data() override;
    dimensions dims() override;
    AVPixelFormat format() override;
};
