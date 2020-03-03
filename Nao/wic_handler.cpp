#include "wic_handler.h"

#include "file_handler_factory.h"

#include "wic_image_provider.h"

file_handler_tag wic_handler::tag() const {
    return TAG_IMAGE;
}

image_provider_ptr wic_handler::make_provider() {
    return std::make_unique<wic_image_provider>(stream);
}

static file_handler_ptr create(const istream_ptr& stream, const std::string& path) {
    return std::make_shared<wic_handler>(stream, path);
}

static bool supports(const istream_ptr& stream, const std::string& path) {
    if (!stream) {
        return false;
    }

    return wic_image_provider::supports(stream);
}

[[maybe_unused]] static size_t id = file_handler_factory::register_class({
    .tag = TAG_IMAGE,
    .creator = create,
    .supports = supports,
    .name = "Windows Imaging Component"
    });
