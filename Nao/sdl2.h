#pragma once

#include <cstdint>

#include <functional>
#include <deque>

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
            // Return the byte array containing the samples
            using callback = std::function<std::vector<char>()>;

            private:
            subsystem_lock _lock { SDL_INIT_AUDIO };

            callback _cb;
            std::function<void(uint8_t*, size_t)> _cb_fwd;
            std::deque<std::vector<char>> _cb_stack;

            SDL_AudioSpec _spec;
            SDL_AudioDeviceID _device;

            public:
            device(int freq, SDL_AudioFormat format,
                uint8_t channels, uint16_t samples, callback callback);
            ~device();

            void pause() const;
            void play() const;

            private:
            void _callback(uint8_t* buffer, size_t len);
        };
    }
}
