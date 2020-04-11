#pragma once

#include <cstdint>

#include <functional>

#include <SDL.h>

namespace sdl {
    inline namespace raii {
        class lock {
            public:
            lock(uint32_t flags = 0);
            ~lock();
        };

        class subsystem_lock {
            uint32_t _flags;

            public:
            subsystem_lock(uint32_t flags = 0);
            ~subsystem_lock();
        };

        class audio_lock {
            SDL_AudioDeviceID _device;

            public:
            audio_lock() = delete;
            audio_lock(SDL_AudioDeviceID device);
            ~audio_lock();
        };
    }

    namespace audio {
        class device {
            public:
            using callback = std::function<void(uint8_t*, int)>;

            private:
            subsystem_lock _lock { SDL_INIT_AUDIO };

            callback _cb;

            SDL_AudioDeviceID _device;

            public:
            device(int freq, SDL_AudioFormat format,
                uint8_t channels, uint16_t samples, callback callback);
            ~device();

            void pause() const;
            void play() const;
        };
    }
}
