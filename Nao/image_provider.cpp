#include "image_provider.h"

size_t image_data::pixel_size(pixel_format type) {
    switch (type) {
        case PIXEL_NONE:   break;
        case PIXEL_RGBA32: return sizeof(pixel_rgba32_t);
        case PIXEL_BGRA32: return sizeof(pixel_bgra32_t);
    }

    return 0;
}

image_data::image_data(image_data&& other) noexcept
    : _data { std::move(other._data) }, _dims { other._dims }, _format { other._format } {
    
}

image_data& image_data::operator=(image_data&& other) noexcept {
    _data = std::move(other._data);
    _dims = other._dims;
    _format = other._format;

    return *this;
}

image_data::image_data(pixel_format type, dimensions size)
    : _data(pixel_size(type) * size.width * size.height)
    , _dims { size }, _format { type } {
    
}

image_data::image_data(pixel_format type, dimensions size, const std::vector<char>& data)
    : _data { data }, _dims { size }, _format { type } {
    ASSERT(data.size() == (pixel_size(type) * size.width * size.height));
}

pixel_format image_data::format() const {
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

size_t image_data::size() const {
    return _data.size();
}

image_data image_data::as(pixel_format type) const {
    if (type == _format) {
        return *this;
    }

    throw std::runtime_error("unsupported pixel format conversion");
}

image_provider::image_provider(const istream_ptr& stream) : stream { stream } {
    
}
