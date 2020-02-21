#include "wem_handler.h"

#include "file_handler_factory.h"

#include "wem_pcm_provider.h"

#include "riff.h"

file_handler_tag wem_handler::tag() const {
    return TAG_PCM;
}

pcm_provider_ptr wem_handler::make_provider() {
    return std::make_shared<wem_pcm_provider>(stream);
}

static file_handler_ptr create(const istream_ptr& stream, const std::string& path) {
    return std::make_shared<wem_handler>(stream, path);
}

static bool supports(const istream_ptr& stream, const std::string& path) {
    if (path.substr(path.size() - 4) == ".wem") {
        wave_header hdr;
        stream->read(&hdr, sizeof(hdr));

        fmt_chunk fmt;
        stream->read(&fmt, sizeof(fmt));

        if (memcmp(hdr.riff.header, "RIFF", 4) == 0 &&
            memcmp(hdr.wave, "WAVE", 4) == 0 &&
            fmt.riff.size == 66 && fmt.format == 0xFFFF) {
            return true;
        }
    }

    return false;
}

[[maybe_unused]] static size_t id = file_handler_factory::register_class({
    .tag = TAG_PCM,
    .creator = create,
    .supports = supports,
    .name = "wem"
    });