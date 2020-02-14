#include "ogg_provider.h"

#include "item_provider_factory.h"
#include "binary_stream.h"
#include "utils.h"
#include "preview.h"

#include <vorbis/vorbisfile.h>

#include <fstream>

ogg_provider::ogg_provider(const istream_type& stream, const std::string& path) : item_provider(stream, path) {
    utils::coutln("[OGG] creating for", path);

    static ov_callbacks callbacks {
        .read_func = [](void* ptr, size_t size, size_t nmemb, void* source) -> size_t {
            ogg_provider* provider = static_cast<ogg_provider*>(source);
            auto& stream = provider->stream;

            if (!stream->good() || stream->eof()) {
                return 0;
            }

            stream->read(ptr, size, nmemb);
            return stream->gcount();
        },/*
        .seek_func = [](void* source, ogg_int64_t offset, int whence) -> int {
            return -1;
            ogg_provider* provider = static_cast<ogg_provider*>(source);
            auto& stream = provider->stream;

            switch (whence) {
                case SEEK_SET: stream->seekg(offset, std::ios::beg); break;
                case SEEK_CUR: stream->seekg(offset, std::ios::cur); break;
                case SEEK_END: stream->seekg(offset, std::ios::end); break;
                default: break;
            }

            stream->clear();

            return 0;
        },*/
        .tell_func = [](void* source) -> long {
            return static_cast<long>(static_cast<ogg_provider*>(source)->stream->tellg());
        }
    };

    utils::coutln(stream->tellg());

    OggVorbis_File vf;
    
    if (ov_open_callbacks(this, &vf, nullptr, 0, callbacks) < 0) {
        throw std::runtime_error("invalid ogg stream");
    }

    vorbis_info* info = ov_info(&vf, -1);

    std::stringstream ss;
    ss << "Channels: " << info->channels << '\n'
        << "Sample rate: " << info->rate << '\n'
        << "Version: " << info->version << '\n'
        << "Lower bitrate: " << info->bitrate_lower << '\n'
        << "Nominal bitrate: " << utils::bits(info->bitrate_nominal) << " / s " << '\n'
        << "Upper bitrate: " << info->bitrate_upper << '\n'
        << "Window bitrate: " << info->bitrate_window << std::endl;
    utils::coutln(ss.str());

    ov_clear(&vf);
}

preview_element_type ogg_provider::preview_type() const {
    return PREVIEW_LIST_VIEW;
}

std::unique_ptr<preview> ogg_provider::make_preview(nao_view& view) {
    //return std::make_unique<audio_player_preview>(view, this);
    return nullptr;
}

static item_provider_ptr create(const item_provider::istream_type& stream, const std::string& path) {
    return std::make_shared<ogg_provider>(stream, path);
}

static bool preview(const item_provider::istream_type& stream, const std::string& path) {
    if (path.substr(path.size() - 4) == ".ogg") {
        std::string fcc(4, '\0');
        stream->read(fcc);

        if (fcc == "OggS") {
            return true;
        }
    }

    return false;
}

static size_t id = item_provider_factory::register_class({
    .creator = create,
    .preview = preview,
    .name = "ogg"
    });
