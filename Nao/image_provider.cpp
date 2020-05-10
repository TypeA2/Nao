#include "image_provider.h"

extern "C" {
#include <libavutil/pixdesc.h>
}

image_data::image_data(AVPixelFormat format, dimensions dims)
    : _format { format }, _dims { dims }
    , _data(dims.width * dims.height
            * (av_get_padded_bits_per_pixel(av_pix_fmt_desc_get(_format)) / CHAR_BIT)) {
    
}

image_data::image_data(AVPixelFormat format, dimensions dims, std::vector<char> data)
    : _format { format }, _dims { dims }, _data { std::move(data) } {
    
}

AVPixelFormat image_data::format() const {
    return _format;
}

dimensions image_data::dims() const {
    return _dims;
}

char* image_data::data() {
    return _data.data();
}

const char* image_data::data() const {
    return _data.data();
}

size_t image_data::bytes() const {
    return _data.size();
}

image_provider::image_provider(istream_ptr stream) : stream { std::move(stream) } {

}

