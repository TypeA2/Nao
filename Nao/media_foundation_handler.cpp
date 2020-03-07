#include  "media_foundation_handler.h"

#include "file_handler_factory.h"

file_handler_tag media_foundation_handler::tag() const {
    return TAG_AV;
}

static file_handler_ptr create(const istream_ptr& stream, const std::string& path) {
    return std::make_shared<media_foundation_handler>(stream, path);
}

static bool supports(const istream_ptr& stream, const std::string& path) {
    if (path.size() > 5 && (path.substr(path.size() - 3) == ".ts" || path.substr(path.size() - 5) == ".mpeg"
        || path.substr(path.size() - 4) == ".mp4")) {
        return true;
    }

    return false;
}


[[maybe_unused]] static size_t id = file_handler_factory::register_class({
    .tag = TAG_AV,
    .creator = create,
    .supports = supports,
    .name = "Media Foundation"
    });
