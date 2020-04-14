#include "wem_pcm_provider.h"

#include "riff.h"
#include "wwriff.h"

#include "utils.h"

#include <ogg/ogg.h>

#include "frameworks.h"
#include "resource.h"

#include "namespaces.h"

#include "vorbis_decoder.h"

namespace detail {
    static istream_ptr decode(const istream_ptr& stream) {
        auto buf = std::make_shared<binary_iostream>(
            std::make_unique<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary));

        stream->seekg(0);
        ASSERT(wwriff::wwriff_to_ogg(stream, buf));

        buf->seekg(0);

        return buf;
    }
}

wem_pcm_provider::wem_pcm_provider(const istream_ptr& stream) : ffmpeg_pcm_provider(detail::decode(stream)) {
    
}
