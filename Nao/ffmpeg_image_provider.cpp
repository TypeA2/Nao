#include "ffmpeg_image_provider.h"

extern "C" {
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

ffmpeg_image_provider::ffmpeg_image_provider(istream_ptr s, const std::string& path)
    : image_provider { std::move(s) }, _ctx { stream, path } {
    auto stream = _ctx.best_stream(AVMEDIA_TYPE_VIDEO);
    ASSERT(stream && stream.id() != AV_CODEC_ID_NONE && stream.codec_frame_count() == 1);
    auto codec = ffmpeg::avcodec::codec { stream.id() };
    auto codec_ctx = ffmpeg::avcodec::context { codec };
    ASSERT(codec_ctx.parameters_to_context(stream));
    ASSERT(codec_ctx.open(codec));

    _fmt = codec_ctx.pix_fmt();
    _dims = {
        .width = codec_ctx.ctx()->width,
        .height = codec_ctx.ctx()->height
    };

    ffmpeg::packet packet;
    ffmpeg::frame frame;

    int res;
    do {
        res = _ctx.read_frame(packet, stream.index());
        ASSERT(res == 0);

        res = codec_ctx.decode(packet, frame);

        if (res != 0 && res != AVERROR(EAGAIN)) {
            throw image_decode_exception("failed to decode frame " + ffmpeg::strerror(res));
        }

        packet.unref();
    } while (res == AVERROR(EAGAIN));

    image_data data { _fmt, _dims, std::vector<char>{ frame.data(), frame.data() + frame.size() } };
    _data = image_data { AV_PIX_FMT_BGRA, _dims };

    SwsContext* sws = sws_getContext(utils::narrow<int>(_dims.width), utils::narrow<int>(_dims.height), _fmt,
      utils::narrow<int>(_dims.width), utils::narrow<int>(_dims.height), AV_PIX_FMT_BGRA,
        SWS_LANCZOS, nullptr, nullptr, nullptr);
    ASSERT(sws);

    AVFrame* av_frame = frame;

    uint8_t* dst_arrs[4];
    int dst_lines[4];
    ASSERT(av_image_fill_arrays(dst_arrs, dst_lines, reinterpret_cast<uint8_t*>(_data.data()), AV_PIX_FMT_BGRA,
        utils::narrow<int>(_dims.width), utils::narrow<int>(_dims.height), 1) > 0);

    ASSERT(sws_scale(sws, av_frame->data, av_frame->linesize, 0,
        utils::narrow<int>(_dims.height),
        dst_arrs, dst_lines) > 0);

    av_image_copy_to_buffer(reinterpret_cast<uint8_t*>(_data.data()), utils::narrow<int>(_data.bytes()), dst_arrs, dst_lines,
        AV_PIX_FMT_BGRA, utils::narrow<int>(_dims.width), utils::narrow<int>(_dims.height), 1);

    sws_freeContext(sws);
}

image_data ffmpeg_image_provider::data() {
    return _data;
}

dimensions ffmpeg_image_provider::dims() {
    return _dims;
}

AVPixelFormat ffmpeg_image_provider::format() {
    return _fmt;
}
