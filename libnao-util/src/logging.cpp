#include "logging.h"

#include <array>

#include <spdlog/sinks/msvc_sink.h>

#include <iconv.h>

namespace detail {
    template <typename Mutex>
    class msvc_unicode_sink : public spdlog::sinks::base_sink<Mutex> {
        public:
        explicit msvc_unicode_sink() { }

        protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

            auto str = fmt::to_string(formatted);
            auto* indata = str.data();
            auto insize = str.size();

            iconv_t conv = iconv_open("UCS-2-INTERNAL", "UTF-8");

            std::vector<wchar_t> result(str.size() + 1, '\0');
            auto* data = result.data();
            auto size = result.size() * sizeof(wchar_t);
            iconv(conv, &indata, &insize,
                reinterpret_cast<char**>(&data), &size);


            OutputDebugStringW(result.data());

            iconv_close(conv);
        }

        void flush_() override { }
    };

    using msvc_unicode_sink_mt = msvc_unicode_sink<std::mutex>;
    using msvc_unicode_sink_st = msvc_unicode_sink<spdlog::details::null_mutex>;
}

namespace nao {
    static std::array sinks{
        //std::make_shared<spdlog::sinks::msvc_sink_mt>()
        std::make_shared<detail::msvc_unicode_sink_mt>()
    };

    spdlog::logger log{ "libnao", sinks.begin(), sinks.end() };

    spdlog::logger make_logger(std::string_view name) {
        return spdlog::logger{
            std::string{ name },
            sinks.begin(), sinks.end()
        };
    }

}
