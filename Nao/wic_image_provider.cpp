#include "wic_image_provider.h"

#include "wic.h"

bool wic_image_provider::supports(const istream_ptr& stream) {
    wic::binary_stream_istream ss(stream);
    auto decoder = wic::imaging_factory().create_decoder(ss);

    return decoder ? true : false;
}


wic_image_provider::wic_image_provider(const istream_ptr& stream)
    : image_provider(stream)
    , _converter { _factory }, _decoder { _factory, this->stream } {

    _frame = _decoder.get_frame(0);

    ASSERT(_converter.can_convert(_frame.pixel_format(), GUID_WICPixelFormat32bppPBGRA));
    ASSERT(_converter.initialize(_frame, GUID_WICPixelFormat32bppPBGRA));

    _dims = _converter.size();

}

image_data wic_image_provider::data() {
    return image_data(PIXEL_BGRA32, _dims, _converter.get_pixels());
}

dimensions wic_image_provider::dims() {
    return _dims;
}

pixel_format wic_image_provider::type() {
    return PIXEL_BGRA32;
}

