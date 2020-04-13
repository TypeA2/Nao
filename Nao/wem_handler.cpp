#include "wem_handler.h"

#include "file_handler_factory.h"

#include "wem_pcm_provider.h"

#include "riff.h"

file_handler_tag wem_handler::tag() const {
    return TAG_PCM;
}

pcm_provider_ptr wem_handler::make_provider() {
    //return std::make_shared<wem_pcm_provider>(stream);
    return nullptr;
}

static file_handler_ptr create(const istream_ptr& stream, const std::string& path) {
    return std::make_shared<wem_handler>(stream, path);
}

static bool supports(const istream_ptr& stream, const std::string& path) {
    if (path.substr(path.size() - 4) == ".wem") {
        riff_header hdr;
        stream->read(&hdr, sizeof(hdr));
        if (std::string(hdr.header, 4) != "RIFF") {
            return false;
        }

        wave_chunk wave;
        stream->read(&wave, sizeof(wave));
        if (std::string(wave.wave, 4) != "WAVE") {
            return false;
        }

        stream->read(&hdr, sizeof(hdr));
        if (std::string(hdr.header, 4) != "fmt ") {
            return false;
        }

        fmt_chunk fmt;
        stream->read(&fmt, sizeof(fmt));

        if (hdr.size == 66 && fmt.format == 0xFFFF) {
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
