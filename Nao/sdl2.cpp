#include "sdl2.h"

#include "utils.h"

#include <numeric>

namespace detail {
    void callback_fwd(void* userdata, uint8_t* buffer, int len) {
        if (len < 0) {
            throw std::runtime_error("requested invalid number of bytes");
        }
        (*static_cast<std::function<void(uint8_t*, size_t)>*>(userdata))(buffer, len);
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
        device::device(int freq, SDL_AudioFormat format, uint8_t channels, uint16_t samples, callback callback)
            : _cb { std::move(callback) }
            , _cb_fwd { std::bind(&device::_callback, this, std::placeholders::_1, std::placeholders::_2) }
            , _spec {
                .freq = freq,
                .format = format,
                .channels = channels,
                .samples = samples,
                .callback = detail::callback_fwd,
                .userdata = &_cb_fwd
            } {
            SDL_AudioSpec obtained;
            _device = SDL_OpenAudioDevice(nullptr, 0, &_spec, &obtained, 0);

            ASSERT(_device != 0 && _spec.format == obtained.format);
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

        void device::_callback(uint8_t* buffer, size_t len) {
            size_t bytes_present = std::accumulate(_cb_stack.begin(), _cb_stack.end(), size_t { 0 },
                [](size_t val, const std::vector<char>& vec) { return val + vec.size(); });

            while (bytes_present < len) {
                _cb_stack.push_back(_cb());
                bytes_present += _cb_stack.back().size();
            }

            size_t bytes_left = len;
            char* cur = reinterpret_cast<char*>(buffer);
            while (bytes_left > 0) {
                auto& data = _cb_stack.front();
                if (bytes_left >= data.size()) {
                    // Whole vector can fit
                    std::copy(data.begin(), data.end(), cur);

                    cur += data.size();

                    bytes_left -= data.size();

                    _cb_stack.pop_front();
                } else {
                    // Truncate this vector
                    std::copy_n(data.begin(), bytes_left, cur);

                    data = std::vector<char> { data.begin() + bytes_left, data.end() };

                    break;
                }
            }
        }
    }
}
