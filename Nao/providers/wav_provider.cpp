#include "wav_provider.h"

#include "utils.h"
#include "audio_player.h"

wav_provider::wav_provider(const file_stream& stream, const std::string& path, data_model& model)
    : item_provider(stream, path, model) {
    utils::coutln("[WAV] creating for", name);
}

wav_provider::~wav_provider() {
    utils::coutln("[WAV] deleting for", name);
}

item_provider::preview_type wav_provider::preview() const {
    return preview_type::PreviewAudioPlayer;
}

ui_element* wav_provider::preview_element(ui_element* parent) const {
    auto player = new audio_player(parent, model);

    player->set_audio(name, file);

    return player;
}


item_provider* wav_provider::_create(const file_stream& file, const std::string& name, data_model& model) {
    if (name.substr(name.size() - 4) == ".wav") {
        std::string fcc(4, '\0');
        file->read(fcc);
        (void) file->read<uint32_t>();

        std::string wavefmt(8, '\0');
        file->read(wavefmt);
        file->rseek(-16);

        if (fcc == "RIFF" && wavefmt == "WAVEfmt ") {
            return new wav_provider(file, name, model);
        }
    }

    return nullptr;
}

size_t wav_provider::_id = item_provider_factory::register_class(_create);
