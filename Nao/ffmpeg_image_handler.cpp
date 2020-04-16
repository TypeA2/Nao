#include "ffmpeg_image_handler.h"

#include "file_handler_factory.h"

#include "ffmpeg.h"

file_handler_tag ffmpeg_image_handler::tag() const {
    return TAG_IMAGE;
}

image_provider_ptr ffmpeg_image_handler::make_provider() {
    return nullptr;
}

static file_handler_ptr create(const istream_ptr& stream, const std::string& path) {
    return nullptr;
}

static bool supports(const istream_ptr& stream, const std::string& path) {
    if (!stream) {
        return false;
    }

    try {
        ffmpeg::avformat::context ctx { stream, path };

        // First stream is audio
        if (ctx.stream_count() > 0) {
            ffmpeg::avformat::stream best_stream = ctx.best_stream(AVMEDIA_TYPE_VIDEO);
            if (best_stream.type() == AVMEDIA_TYPE_VIDEO &&
                best_stream.id() != AV_CODEC_ID_NONE &&
                best_stream.codec_frame_count() == 1) {
                return true;
            }
        }
    } catch (const std::runtime_error&) {

    }

    return false;
}

[[maybe_unused]] static size_t id = file_handler_factory::register_class({
    .tag = TAG_IMAGE,
    .creator = create,
    .supports = supports,
    .name = "ffmpeg image"
    });
