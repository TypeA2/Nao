#include "jpeg_handler.h"

#include "file_handler_factory.h"

file_handler_tag jpeg_handler::tag() const {
    return TAG_IMAGE;
}

image_provider_ptr jpeg_handler::make_provider() {
    return nullptr;
}

static file_handler_ptr create(const istream_ptr& stream, const std::string& path) {
    return std::make_shared<jpeg_handler>(stream, path);
}

static bool supports(const istream_ptr& stream, const std::string& path) {
    auto off = path.find_last_of('.');
    if (off == std::string::npos) {
        return false;
    }

    std::string ext = path.substr(off);
    
    if (ext == ".jpeg" || ext == ".jpg" || ext == ".jpe" || ext == ".jfif" || ext == ".jif") {
        uint8_t buf[2];
        stream->read(buf);

        // Begin marker
        if (buf[0] == 0xff && buf[1] == 0xd8) {
            stream->seekg(-2, std::ios::end);
            stream->read(buf);
            if (buf[0] == 0xff && buf[1] == 0xd9) {
                return true;
            }
        }
    }

    return false;
}

[[maybe_unused]] static size_t id = file_handler_factory::register_class({
    .tag = TAG_IMAGE,
    .creator = create,
    .supports = supports,
    .name = "JPEG"
    });
