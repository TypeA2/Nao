#include "ogg_provider.h"

#include "item_provider_factory.h"
#include "binary_stream.h"
#include "utils.h"
#include "preview.h"

#include "audio_player.h"

ogg_provider::ogg_provider(const istream_type& stream, const std::string& path) : item_provider(stream, path) {
    utils::coutln("[OGG] creating for", path);
    /*auto err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error("Pa_Initialize error");
    }

    

    

    static ov_callbacks callbacks {
        .read_func = [](void* ptr, size_t size, size_t nmemb, void* source) -> size_t {
            ogg_provider* provider = static_cast<ogg_provider*>(source);
            auto& stream = provider->stream;

            if (!stream->good() || stream->eof()) {
                return 0;
            }

            stream->read(ptr, size, nmemb);
            return stream->gcount();
        }
    };

    utils::coutln(stream->tellg());

    OggVorbis_File vf;
    
    if (ov_open_callbacks(this, &vf, nullptr, 0, callbacks) < 0) {
        Pa_Terminate();
        throw std::runtime_error("invalid ogg stream");
    }

    vorbis_info* info = ov_info(&vf, -1);

    std::stringstream ss;
    ss << "Channels: " << info->channels << '\n'
        << "Sample rate: " << info->rate << '\n'
        << "Version: " << info->version << '\n'
        << "Lower bitrate: " << info->bitrate_lower << '\n'
        << "Nominal bitrate: " << utils::bits(info->bitrate_nominal) << " / s " << '\n'
        << "Upper bitrate: " << info->bitrate_upper << '\n'
        << "Window bitrate: " << info->bitrate_window << std::endl;
    utils::coutln(ss.str());

    PaStream* pa_stream;

    int default_device = Pa_GetDefaultOutputDevice();
    auto default_info = Pa_GetDeviceInfo(default_device);
    PaStreamParameters params {
        .device = default_device,
        .channelCount = info->channels,
        .sampleFormat = paFloat32,
        .suggestedLatency = default_info->defaultHighOutputLatency
    };

    err = Pa_OpenStream(&pa_stream, nullptr, &params, default_info->defaultSampleRate, 1024, paClipOff, nullptr, nullptr);
    if (err != paNoError) {
        utils::coutln("PA error", Pa_GetErrorText(err));
        Pa_Terminate();
        throw std::runtime_error("Pa_OpenDefaultStream error");
    }

    err = Pa_StartStream(pa_stream);
    if (err != paNoError) {
        utils::coutln("PA error", Pa_GetErrorText(err));
        Pa_Terminate();
        throw std::runtime_error("Pa_StartStream error");
    }

    cb_state state {
        .file = &vf,
        .info = info
    };

    static src_callback_t cb = [](void* data, float** buf) -> long {
        auto state = static_cast<cb_state*>(data);
        char pcm[4096];
        int bitstream;
        // Get bytes read
        long size = ov_read(state->file, pcm, sizeof(pcm), 0, 2, 1, &bitstream);
        if (size == 0) {
            return 0;
        }

        if (size < 0) {
            Pa_Terminate();
            throw std::runtime_error("decode error");
        }

        long frames = size / 2 / state->info->channels;
        short* pcm_sht = reinterpret_cast<short*>(pcm);
        *buf = new float[frames * state->info->channels];
        src_short_to_float_array(pcm_sht, *buf, frames * state->info->channels);

        return frames;
    };
    int src_err;
    SRC_STATE* src_state = src_callback_new(cb, SRC_SINC_MEDIUM_QUALITY, info->channels, &src_err, &state);


    double ratio = default_info->defaultSampleRate / static_cast<double>(info->rate);

    float pcm_flt[4096];
    while (true) {
        
        long frames = src_callback_read(src_state, ratio, 1024, pcm_flt);

        err = Pa_WriteStream(pa_stream, pcm_flt, frames);
        if (err) {
            utils::coutln("error!");
            break;
        }
    }

    err = Pa_CloseStream(pa_stream);
    if (err != paNoError) {
        utils::coutln("PA close err");
    }

    ov_clear(&vf);

    src_delete(src_state);

    err = Pa_Terminate();
    if (err != paNoError) {
        throw std::runtime_error("Pa_Terminate error");
    }*/
}

preview_element_type ogg_provider::preview_type() const {
    return PREVIEW_PLAY_AUDIO;
}

std::unique_ptr<preview> ogg_provider::make_preview(nao_view& view) {
    return std::make_unique<audio_player_preview>(view, this);
}

static item_provider_ptr create(const item_provider::istream_type& stream, const std::string& path) {
    return std::make_shared<ogg_provider>(stream, path);
}

static bool preview(const item_provider::istream_type& stream, const std::string& path) {
    if (path.substr(path.size() - 4) == ".ogg") {
        std::string fcc(4, '\0');
        stream->read(fcc);

        if (fcc == "OggS") {
            return true;
        }
    }

    return false;
}

static size_t id = item_provider_factory::register_class({
    .creator = create,
    .preview = preview,
    .name = "ogg"
    });
