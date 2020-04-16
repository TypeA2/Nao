#pragma once

#include "binary_stream.h"

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavformat/avformat.h>
}

struct AVFormatContext;
struct AVIOContext;
struct AVStream;

namespace ffmpeg {
    inline namespace generic {
        std::string strerror(int err);

        class frame {
            AVFrame* _frame;

            public:
            frame();
            ~frame();

            uint64_t samples() const;
            uint8_t channels() const;

            const char* data(size_t index = 0) const;

            const char* operator[](size_t index) const;

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

            void unref() const;
        };
    }

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

            operator bool() const;

            operator AVStream* () const;
        };

        class context {
            AVFormatContext* _ctx;
            istream_ptr _stream;

            avio::io_context _ioctx;

            std::vector<stream> _streams;

            public:
            explicit context(istream_ptr stream, const std::string& path = "");
            ~context();

            AVFormatContext* ctx() const;

            size_t stream_count() const;
            const std::vector<stream>& streams() const;

            // Return the first stream of the specified type
            stream first_stream(AVMediaType type) const;

            int read_frame(packet& pkt) const;

            // Read and discard until the next packet with the specified index;
            int read_frame(packet& pkt, int index) const;

            bool seek(std::chrono::nanoseconds pos, int index);
        };
    }

    namespace avcodec {
        class codec {
            AVCodec* _codec;
            public:
            explicit codec(AVCodecID id);
            explicit codec(AVCodec* codec);
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
            int64_t sample_rate() const;

            // Send and receive
            int decode(const packet& pkt, frame& frame) const;

            AVCodecContext* ctx() const;
            uint64_t channel_layout() const;
            uint8_t channels() const;
        };
    }

    namespace swresample {
        class context {
            public:
            struct audio_info {
                int64_t channel_layout;
                AVSampleFormat sample_format;
                int64_t sample_rate;
            };

            private:
            SwrContext* _swr;

            audio_info _in;
            audio_info _out;

            int _bytes_per_out_sample;

            public:
            context(const audio_info& in, const audio_info& out);
            ~context();

            int64_t frames_for_input(int64_t frames) const;

            int64_t convert(char** in, int64_t in_frames, char** out, int64_t out_frames) const;
        };
    }
}
