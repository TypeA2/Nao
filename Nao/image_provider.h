#pragma once

#include "binary_stream.h"

extern "C" {
#include <libavutil/pixfmt.h>
}

class image_data final {
    AVPixelFormat _format;
    dimensions _dims;
    std::vector<char> _data { };

    public:
    image_data() = default;
    image_data(AVPixelFormat format, dimensions dims);
    image_data(AVPixelFormat format, dimensions dims, std::vector<char> data);

    AVPixelFormat format() const;
    dimensions dims() const;

    char* data();
    const char* data() const;

    size_t bytes() const;

    template <concepts::iterator<char> InputIt>
    size_t fill(InputIt begin, InputIt end) {
        if (std::distance(begin, end) > _data.size()) {
            throw std::out_of_range("tried to fill past end");
        }

        return std::distance(_data.begin(),
            std::copy(begin, end, _data.begin()));
    }

    template <concepts::iterator<char> InputIt>
    size_t fill_n(InputIt begin, size_t count) {
        if (count > _data.size()) {
            throw std::out_of_range("tried to fill past end");
        }

        return std::distance(_data.begin(),
            std::copy_n(begin, count, _data.begin()));
    }

    /*
    image_data() = delete;
    image_data(image_data&& other) noexcept;
    image_data& operator=(image_data&& other) noexcept;
    image_data(pixel_format type, dimensions size);
    image_data(pixel_format type, dimensions size, const std::vector<char>& data);

    pixel_format format() const;
    dimensions dims() const;

    template <pixel_format type>
    pixel_type_t<type>* data() {
        return reinterpret_cast<pixel_type_t<type>*>(_data.data());
    }

    template <pixel_format type>
    const pixel_type_t<type>* data() const {
        return reinterpret_cast<const pixel_type_t<type>*>(_data.data());
    }

    char* data();
    const char* data() const;

    // size in bytes
    size_t size() const;

    image_data as(pixel_format type) const;

    private:
    image_data(const image_data& other) = default;
    image_data& operator=(const image_data& other) = default;*/
};

class image_provider {
    protected:
    istream_ptr stream;

    public:
    explicit image_provider(istream_ptr stream);
    virtual ~image_provider() = default;

    virtual image_data data() = 0;
    virtual dimensions dims() = 0;
    virtual AVPixelFormat format() = 0;
};

using image_provider_ptr = std::unique_ptr<image_provider>;

class image_decode_exception : public std::runtime_error {
    public:
    using std::runtime_error::runtime_error;
};
