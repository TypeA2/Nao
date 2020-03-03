#pragma once

#include "image_provider.h"

class wic_image_provider : public image_provider {
    dimensions _dims;

    std::vector<char> _data;

    public:
    static bool supports(const istream_ptr& stream);

    explicit wic_image_provider(const istream_ptr& stream);
    ~wic_image_provider() override = default;

    image_data data() override;
    dimensions dims() override;
    pixel_format type() override;
};
