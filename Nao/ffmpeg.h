#pragma once

#include "binary_stream.h"

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}

struct AVFormatContext;
struct AVIOContext;
struct AVStream;

namespace ffmpeg {
    class frame {
        AVFrame* _frame;

        public:
        frame();
        ~frame();

        int64_t samples() const;
        int64_t channels() const;

        const char* data(size_t index = 0) const;
        size_t size(size_t index = 0) const;

        operator AVFrame* () const noexcept;
    };

    class packet {
        AVPacket* _packet;

        public:
        packet();
        ~packet();

        int stream_index() const;

        operator AVPacket* () const noexcept;
    };
    namespace avio {
        class io_context {
            static constexpr size_t buffer_size = 4096;

            istream_ptr _stream;
            AVIOContext* _ctx;

            public:
            explicit io_context(const istream_ptr& stream);
            ~io_context();

            AVIOContext* ctx() const;
        };
    }

    namespace avformat {
        class stream {
            AVStream* _stream;

            public:
            explicit stream(AVStream* stream);
            stream() = default;

            AVMediaType type() const;
            AVCodecID id() const;
            int index() const;

            int64_t sample_rate() const;
            int64_t channels() const;

            std::chrono::nanoseconds duration() const;
            double time_base() const;

            AVCodecParameters* params() const;
        };

        class context {
            AVFormatContext* _ctx;
            istream_ptr _stream;

            avio::io_context _ioctx;

            std::vector<stream> _streams;

            public:
            explicit context(const istream_ptr& stream, const std::string& path = "");
            ~context();

            AVFormatContext* ctx() const;

            size_t stream_count() const;
            const std::vector<stream>& streams() const;

            bool read_frame(packet& pkt) const;
            // Read and discard until the next packet with the specified index;
            bool read_frame(packet& pkt, int index) const;

            bool seek(std::chrono::nanoseconds pos, int index);
        };
    }

    namespace avcodec {
        class codec {
            AVCodec* _codec;
            public:
            explicit codec(AVCodecID id);
            codec() = default;

            AVCodec* ctx() const;

            std::string name() const;
            std::string long_name() const;
        };

        class context {
            AVCodecContext* _ctx;

            public:
            explicit context(const codec& codec);
            context();

            context(const context&) = delete;
            context& operator=(const context&) = delete;
            context(context&& other) noexcept;
            context& operator=(context&& other) noexcept;
            
            ~context();

            bool parameters_to_context(const avformat::stream& stream) const;
            bool open(const codec& codec) const;

            AVSampleFormat sample_format() const;

            // Send and receive
            bool decode(const packet& pkt, frame& frame) const;

            AVCodecContext* ctx() const;
        };
    }
}
