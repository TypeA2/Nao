#include "wav_provider.h"

#include "item_provider_factory.h"
#include "binary_stream.h"
#include "utils.h"
#include "preview.h"

wav_provider::wav_provider(const istream_type& stream, const std::string& path) : item_provider(stream, path) {
    utils::coutln("[WAV] creating for", path);
}

preview_element_type wav_provider::preview_type() const {
    return PREVIEW_PLAY_AUDIO;
}

std::unique_ptr<preview> wav_provider::make_preview(nao_view& view) {
    return std::make_unique<audio_player_preview>(view, this);
}

static item_provider_ptr create(const item_provider::istream_type& stream, const std::string& path) {
    if (path.substr(path.size() - 4) == ".wav") {
        std::string fcc(4, '\0');
        stream->read(fcc);
        (void) stream->read<uint32_t>();

        std::string wavefmt(8, '\0');
        stream->read(wavefmt);
        stream->rseek(-16);

        if (fcc == "RIFF" && wavefmt == "WAVEfmt ") {
            return std::make_shared<wav_provider>(stream, path);
        }
    }

    return nullptr;
}

static size_t id = item_provider_factory::register_class(create, "wav");
