#include "flac_handler.h"

#include "file_handler_factory.h"

#include "flac_pcm_provider.h"

file_handler_tag flac_handler::tag() const {
    return TAG_PCM;
}

pcm_provider_ptr flac_handler::make_provider() {
    //return std::make_shared<flac_pcm_provider>(stream);
    return nullptr;
}

static file_handler_ptr create(const istream_ptr& stream, const std::string& path) {
    return std::make_shared<flac_handler>(stream, path);
}

static bool supports(const istream_ptr& stream, const std::string& path) {
    if (path.substr(path.size() - 5) == ".flac") {
        std::string fcc(4, '\0');
        stream->read(fcc);

        if (fcc == "fLaC") {
            return true;
        }
    }

    return false;
}

[[maybe_unused]] static size_t id = file_handler_factory::register_class({
    .tag = TAG_PCM,
    .creator = create,
    .supports = supports,
    .name = "flac"
    });