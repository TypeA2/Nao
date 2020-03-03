#pragma once

#include "binary_stream.h"

enum pixel_format : uint64_t {
    PIXEL_NONE,
    PIXEL_RGBA32,
    PIXEL_BGRA32
};

using pixel_rgba32_t = uint32_t;
using pixel_bgra32_t = uint32_t;

template <pixel_format> struct pixel_type { };
template <> struct pixel_type<PIXEL_RGBA32> { using type = pixel_rgba32_t; };
template <> struct pixel_type<PIXEL_BGRA32> { using type = pixel_bgra32_t; };

template <pixel_format fmt>
using pixel_type_t = typename pixel_type<fmt>::type;

class image_data final {
    public:
    static size_t pixel_size(pixel_format type);

    private:
    std::vector<char> _data;
    dimensions _dims;
    pixel_format _format;

    public:
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
    image_data& operator=(const image_data& other) = default;
};

class image_provider {
    protected:
    istream_ptr stream;

    public:
    explicit image_provider(const istream_ptr& stream);
    virtual ~image_provider() = default;

    virtual image_data data() = 0;
    virtual dimensions dims() = 0;
    virtual pixel_format type() = 0;
};

using image_provider_ptr = std::unique_ptr<image_provider>;
