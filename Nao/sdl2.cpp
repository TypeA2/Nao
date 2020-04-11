#include "sdl2.h"

#include "utils.h"

namespace detail {
    void callback_fwd(void* userdata, uint8_t* buffer, int len) {
        (*static_cast<sdl::audio::device::callback*>(userdata))(buffer, len);
    }
}

namespace sdl {
    inline namespace raii {
        lock::lock(uint32_t flags) {
            ASSERT(SDL_Init(flags) == 0);
        }

        lock::~lock() {
            SDL_Quit();
        }

        subsystem_lock::subsystem_lock(uint32_t flags) : _flags { flags } {
            ASSERT(SDL_InitSubSystem(_flags) == 0);
        }

        subsystem_lock::~subsystem_lock() {
            SDL_QuitSubSystem(_flags);
        }

        audio_lock::audio_lock(SDL_AudioDeviceID device) : _device { device } {
            SDL_LockAudioDevice(_device);
        }

        audio_lock::~audio_lock() {
            SDL_UnlockAudioDevice(_device);
        }


    }

    namespace audio {
        device::device(int freq, SDL_AudioFormat format, uint8_t channels, uint16_t samples, callback callback) : _cb { std::move(callback) } {
            SDL_AudioSpec spec {
                .freq = freq,
                .format = format,
                .channels = channels,
                .samples = samples,
                .callback = detail::callback_fwd,
                .userdata = &_cb
            };

            SDL_AudioSpec obtained;
            _device = SDL_OpenAudioDevice(nullptr, 0, &spec, &obtained, 0);

            ASSERT(_device != 0 && spec.format == obtained.format);
        }

        device::~device() {
            pause();
            SDL_CloseAudioDevice(_device);
        }

        void device::pause() const {
            SDL_PauseAudioDevice(_device, 1);
        }

        void device::play() const {
            SDL_PauseAudioDevice(_device, 0);
        }
    }
}
