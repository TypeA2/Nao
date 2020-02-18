#include "audio_player.h"

#include "thread_pool.h"

#include <portaudio.h>
#include <samplerate.h>

class audio_player_pa_lock {
    public:
    explicit audio_player_pa_lock() {
        if (Pa_Initialize() != paNoError) {
            throw std::runtime_error("Pa_Initialize error");
        }
    }

    ~audio_player_pa_lock() {
        if (Pa_Terminate() != paNoError) {
            utils::coutln("Pa_Terminate error");
        }
    }
};

audio_player::audio_player(pcm_provider_ptr provider) : _d { std::make_unique<audio_player_pa_lock>() }
    , _m_provider { std::move(provider) }, _m_player(1)
    , _m_convert_rate { false }
    , _m_quit { false }, _m_pause { true }, _m_eof { false }
    , _m_volume { 1.f } {
   
    reset();
}

audio_player::~audio_player() {
    _m_quit = true;
    _m_pause = false;
    _m_pause_condition.notify_all();

    trigger_event(EVENT_STOP);
}

std::chrono::nanoseconds audio_player::duration() const {
    return _m_provider->duration();
}

std::chrono::nanoseconds audio_player::pos() const {
    return _m_provider->pos();
}

void audio_player::seek(std::chrono::nanoseconds pos) const {
    _m_provider->seek(pos);
}

bool audio_player::paused() const {
    return _m_pause;
}

bool audio_player::eof() const {
    return _m_eof;
}

void audio_player::pause() {
    _m_pause = true;
}

void audio_player::play() {
    _m_pause = false;
    _m_pause_condition.notify_all();
}

void audio_player::reset() {
    _m_eof = false;
    _m_pause = true;

    _m_provider->seek(std::chrono::nanoseconds(0));

    PaDeviceIndex device = Pa_GetDefaultOutputDevice();
    auto info = Pa_GetDeviceInfo(device);

    sample_type supported_samples = _m_provider->types();
    sample_type output_format = _m_provider->preferred_type();

    // If we need to convert the sample rate
    if (_m_provider->rate() != info->defaultSampleRate) {
        if (!(supported_samples & SAMPLE_FLOAT32)) {
            // float32 output is not supported, first convert it, then just output the float32 to PortAudio
            _m_convert_format = true;
        }
        _m_convert_rate = true;
        output_format = SAMPLE_FLOAT32;
    }

    PaStreamParameters params {
        .device = device,
        .channelCount = static_cast<int>(_m_provider->channels()),
        .sampleFormat = pcm_provider::pa_format(output_format),
        .suggestedLatency = info->defaultHighOutputLatency
    };

    ASSERT(paNoError == Pa_OpenStream(&_m_stream, nullptr, &params, info->defaultSampleRate, paFramesPerBufferUnspecified, paClipOff, nullptr, nullptr));
    ASSERT(paNoError == Pa_StartStream(_m_stream));

    _m_startup_done = false;

    if (_m_convert_rate) {
        _m_player.push(&audio_player::_playback_loop_resample, this, info);
    } else {
        // No sample rate conversion, basic playback
        _m_player.push(&audio_player::_playback_loop_passthrough, this, output_format);
    }

    std::unique_lock lock(_m_startup_mutex);
    _m_startup_condition.wait(lock, [this]() -> bool { return _m_startup_done; });
}

void audio_player::set_volume_scaled(float val) {
    _m_volume = std::clamp(val, 0.f, 1.f);
}

void audio_player::set_volume_log(float orig, float curve) {
    _m_volume = std::clamp(pow(orig, curve), 0.f, 1.f);
}

float audio_player::volume_scaled() const {
    return _m_volume;
}

float audio_player::volume_log(float curve) const {
    return std::clamp(pow(_m_volume, 1.f / curve), 0.f, 1.f);
}

void audio_player::add_event(event_type type, const event_handler& handler) {
    if (handler) {
        _m_events[type].push_back(handler);
    }
}

void audio_player::trigger_event(event_type type) const {
    if (_m_events.find(type) == _m_events.end()) {
        return;
    }

    for (const event_handler& func : _m_events.at(type)) {
        func();
    }
}

void audio_player::_playback_loop_passthrough(sample_type output_format) {
    const auto channels = _m_provider->channels();

    while (true) {
        // Wait until pause is over
        if (_m_pause) {
            if (!_m_startup_done) {
                _m_startup_done = true;
                _m_startup_condition.notify_all();
            } else {
                trigger_event(EVENT_STOP);
            }

            std::unique_lock lock(_m_pause_mutex);
            _m_pause_condition.wait(lock, [this] { return !_m_pause; });

            if (!_m_quit) {
                trigger_event(EVENT_START);
            }
        }

        if (_m_quit) {
            break;
        }

        void* samples;
        int64_t frames = _m_provider->get_samples(samples, output_format);
        if (frames <= 0) {
            break;
        }

        _write_samples(samples, frames, channels, output_format);
        pcm_provider::delete_samples(samples, output_format);
    }

    _m_pause = true;
    _m_eof = true;

    if (!_m_quit) {
        trigger_event(EVENT_STOP);
    }

    if (_m_stream) {
        ASSERT(paNoError == Pa_CloseStream(_m_stream));
    }
}

void audio_player::_playback_loop_resample(const PaDeviceInfo* info) {
    static constexpr size_t buf_size = 2048;
    float pcm[buf_size];
    const double conversion_ratio = info->defaultSampleRate / static_cast<double>(_m_provider->rate());
    const int64_t channels = _m_provider->channels();

    const long frames = static_cast<long>(buf_size / channels);

    struct cb_args {
        pcm_provider* provider;
        float* prev_buf;
        bool convert_format;
        sample_type src_type;
        int64_t channels;
    } args {
        .provider = _m_provider.get(),
        .convert_format = _m_convert_format,
        .src_type = SAMPLE_FLOAT32,
        .channels = channels
    };

    if (_m_convert_format) {
        args.src_type = _m_provider->preferred_type();
    }

    src_callback_t src_callback = [](void* data, float** samples) -> long {
        auto& [provider, prev_buf, convert, src_type, channels] = *static_cast<cb_args*>(data);
        
        void* pcm;
        long frames = static_cast<long>(provider->get_samples(pcm, src_type));

        if (frames == 0) {
            return 0;
        }

        pcm_provider::delete_samples(prev_buf, SAMPLE_FLOAT32);

        // Convert sample rate if needed
        if (convert) {
            void* pcm_conv;
            pcm_provider::convert_samples(pcm, src_type, pcm_conv, SAMPLE_FLOAT32, frames, channels);
            pcm_provider::delete_samples(pcm, src_type);

            pcm = pcm_conv;
        }

        *samples = static_cast<float*>(pcm);
        prev_buf = *samples;
        return frames;
    };

    int conversion_err;
    SRC_STATE* state = src_callback_new(src_callback, SRC_SINC_MEDIUM_QUALITY,
        static_cast<int>(channels), &conversion_err, &args);

    while (true) {
        // Wait until pause is over
        if (_m_pause) {
            if (!_m_startup_done) {
                _m_startup_done = true;
                _m_startup_condition.notify_all();
            } else {
                trigger_event(EVENT_STOP);
            }

            std::unique_lock lock(_m_pause_mutex);
            _m_pause_condition.wait(lock, [this] { return !_m_pause; });

            if (!_m_quit) {
                trigger_event(EVENT_START);
            }
        }

        if (_m_quit) {
            break;
        }

        // Convert sample rate
        long frames_converted = src_callback_read(state, conversion_ratio, frames, pcm);
        if (frames_converted <= 0) {
            break;
        }

        _write_samples(pcm, frames_converted, channels, SAMPLE_FLOAT32);
    }

    _m_pause = true;
    _m_eof = true;

    if (!_m_quit) {
        trigger_event(EVENT_STOP);
    }

    if (_m_stream) {
        ASSERT(paNoError == Pa_CloseStream(_m_stream));
    }

    src_delete(state);
}

inline void audio_player::_write_samples(void* samples, int64_t frames, int64_t channels, sample_type type) const {
    int64_t sample_count = frames * channels;

    // Apply volume scaling
    switch (type) {
        case SAMPLE_INT16: {
            short* data = static_cast<short*>(samples);
            for (int64_t i = 0; i < sample_count; ++i) {
                data[i] = static_cast<short>(static_cast<float>(data[i]) * _m_volume);
            }
            break;
        }

        case SAMPLE_FLOAT32: {
            float* data = static_cast<float*>(samples);
            for (int64_t i = 0; i < sample_count; ++i) {
                data[i] *= _m_volume;
            }
            break;
        }
        case SAMPLE_NONE: break;
    }

    Pa_WriteStream(_m_stream, samples, static_cast<int>(frames));
}
