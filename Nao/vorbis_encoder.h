#pragma once

#include <vorbis/vorbisenc.h>

class vorbis_encoder {
    vorbis_info _vi;
    vorbis_comment _vc;

    public:
    vorbis_encoder();
    ~vorbis_encoder();

    bool headerin(const ogg_packet& packet);

    long blocksize(const ogg_packet& packet);
};
