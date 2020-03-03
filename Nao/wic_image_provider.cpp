#include "wic_image_provider.h"

#include "wic.h"

bool wic_image_provider::supports(const istream_ptr& stream) {
    wic::binary_stream_istream ss(stream);
    auto decoder = wic::imaging_factory().create_decoder(ss);

    return decoder ? true : false;
}


wic_image_provider::wic_image_provider(const istream_ptr& stream) : image_provider(stream) {
    wic::imaging_factory factory;
    wic::bitmap_decoder decoder(factory, this->stream);

    wic::bitmap_frame_decode frame = decoder.get_frame(0);
    wic::format_converter converter { factory };

    ASSERT(converter.can_convert(frame.pixel_format(), GUID_WICPixelFormat32bppPBGRA));
    ASSERT(converter.initialize(frame, GUID_WICPixelFormat32bppPBGRA));

    _dims = converter.size();

    _data = converter.get_pixels();
    ASSERT(!_data.empty());
}

image_data wic_image_provider::data() {
    return image_data(PIXEL_BGRA32, _dims, _data);
}

dimensions wic_image_provider::dims() {
    return _dims;
}

pixel_format wic_image_provider::type() {
    return PIXEL_BGRA32;
}

