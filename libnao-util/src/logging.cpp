#include "logging.h"

#include <array>

#include <Windows.h>

#include <spdlog/sinks/base_sink.h>

#include <encoding.h>

namespace detail {
    template <typename Mutex>
    class msvc_unicode_sink : public spdlog::sinks::base_sink<Mutex> {
        public:
        explicit msvc_unicode_sink() = default;
        ~msvc_unicode_sink() override = default;

        protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);


            OutputDebugStringW(nao::utf8_to_wide(fmt::to_string(formatted)).c_str());
        }

        void flush_() override { }
    };

    using msvc_unicode_sink_mt = msvc_unicode_sink<std::mutex>;
    using msvc_unicode_sink_st = msvc_unicode_sink<spdlog::details::null_mutex>;
}

namespace nao {
    static std::array sinks{
        std::make_shared<detail::msvc_unicode_sink_mt>()
    };


    spdlog::logger log{ "libnao", sinks.begin(), sinks.end() };


    spdlog::logger make_logger(std::string_view name) {
        return spdlog::logger{
            std::string{ name },
            sinks.begin(), sinks.end()
        };
    }

    spdlog::logger make_logger(std::string_view name, spdlog::level::level_enum level) {
        auto l = make_logger(name);
        l.set_level(level);

        return l;
    }
}
