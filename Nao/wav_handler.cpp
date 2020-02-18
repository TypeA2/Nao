#include "wav_handler.h"

#include "file_handler_factory.h"
#include "binary_stream.h"

file_handler_tag wav_handler::tag() const {
    return TAG_PCM;
}

pcm_provider_ptr wav_handler::make_provider() {
    return nullptr;
}

static file_handler_ptr create(const istream_ptr& stream, const std::string& path) {
    return std::make_shared<wav_handler>(stream, path);
}

static bool supports(const istream_ptr& stream, const std::string& path) {
    if (path.substr(path.size() - 4) == ".wav") {
        std::string fcc(4, '\0');
        stream->read(fcc);
        (void) stream->read<uint32_t>();

        std::string wavefmt(8, '\0');
        stream->read(wavefmt);
        stream->rseek(-16);

        if (fcc == "RIFF" && wavefmt == "WAVEfmt ") {
            return true;
        }
    }

    return false;
}

[[maybe_unused]] static size_t id = file_handler_factory::register_class({
    .tag = TAG_PCM,
    .creator = create,
    .supports = supports,
    .name = "wav"
});
