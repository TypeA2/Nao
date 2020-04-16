#include "wem_pcm_provider.h"

#include "riff.h"
#include "wwriff.h"

#include "utils.h"

#include <fstream>

namespace detail {
    static istream_ptr decode(const istream_ptr& stream) {
        auto buf = std::make_shared<binary_iostream>(
            std::make_unique<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary));

        stream->seekg(0);

        riff_header riff;
        stream->read(&riff, sizeof(riff));
        ASSERT(stream->gcount() == sizeof(riff));
        ASSERT(std::string_view(riff.header, 4) == "RIFF");

        wave_chunk wave;
        stream->read(&wave, sizeof(wave));
        ASSERT(stream->gcount() == sizeof(wave));
        ASSERT(std::string_view(wave.wave, 4) == "WAVE");

        riff_header fmt_riff;
        stream->read(&fmt_riff, sizeof(fmt_riff));
        ASSERT(stream->gcount() == sizeof(fmt_riff));
        ASSERT(std::string_view(fmt_riff.header, 4) == "fmt " && (fmt_riff.size == 24 || fmt_riff.size == 66));

        fmt_chunk fmt;
        stream->read(&fmt, sizeof(fmt));
        ASSERT(stream->gcount() == sizeof(fmt));

        

        switch (fmt.format) {
            case 0xFFFF:
                stream->seekg(0);
                ASSERT(wwriff::wwriff_to_ogg(stream, buf));
                break;
            case 0xFFFE: {
                riff.size += 16;
                fmt_riff.size += 16;

                buf->write(&riff, sizeof(riff));
                buf->write(&wave, sizeof(wave));
                buf->write(&fmt_riff, sizeof(fmt_riff));
                buf->write(&fmt, sizeof(fmt));

                uint16_t extra_size = stream->read<uint16_t>();
                extra_size += 16;
                buf->write(extra_size);

                uint16_t valid_bits = stream->read<uint16_t>();
                buf->write(valid_bits);
                [[maybe_unused]] uint32_t channel_mask = stream->read<uint32_t>();
                buf->write((1ui32 << fmt.channels) - 1);

                uint8_t guid[16] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 };
                buf->write(&guid, sizeof(guid));

                char block[4096];
                while (!stream->eof()) {
                    stream->read(block);

                    size_t bytes = std::min<size_t>(4096, stream->gcount());
                    buf->write(block, bytes);
                }
                break;
            }

            default: break;
        }

        buf->seekg(0);

        return buf;
    }
}

wem_pcm_provider::wem_pcm_provider(const istream_ptr& stream) : ffmpeg_pcm_provider(detail::decode(stream)) {
    
}
