#pragma once

#include "ffmpeg_pcm_provider.h"

class wem_pcm_provider : public ffmpeg_pcm_provider {
    public:
    explicit wem_pcm_provider(const istream_ptr& stream);
};
