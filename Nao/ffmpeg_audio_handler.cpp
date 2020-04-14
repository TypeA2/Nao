#include "ffmpeg_audio_handler.h"

#include "file_handler_factory.h"

#include "ffmpeg_pcm_provider.h"

file_handler_tag ffmpeg_audio_handler::tag() const {
    return TAG_PCM;
}

pcm_provider_ptr ffmpeg_audio_handler::make_provider() {
    return std::make_shared<ffmpeg_pcm_provider>(stream, path);
}

static file_handler_ptr create(const istream_ptr& stream, const std::string& path) {
    return std::make_shared<ffmpeg_audio_handler>(stream, path);
}

static bool supports(const istream_ptr& stream, const std::string& path) {
    if (!stream) {
        return false;
    }

    try {
        ffmpeg::avformat::context ctx { stream, path };

        // First stream is audio
        return ctx.stream_count() > 0
        && ctx.streams()[0].type() == AVMEDIA_TYPE_AUDIO
        && ctx.streams()[0].params()->codec_id != AV_CODEC_ID_NONE;
    } catch (const std::runtime_error&) {
        
    }

    return false;
}

[[maybe_unused]] static size_t id = file_handler_factory::register_class({
    .tag = TAG_PCM,
    .creator = create,
    .supports = supports,
    .name = "ffmpeg audio"
    });
