#pragma once

#include <vorbis/vorbisenc.h>
#include <string>

class vorbis_encoder {
    vorbis_info _vi;
    vorbis_comment _vc;

    int _status = 0;

    public:
    vorbis_encoder();
    ~vorbis_encoder();

    int status() const;

    bool headerin(const ogg_packet& packet);
    void add_tag(const std::string& tag, const std::string& contents);

    long blocksize(const ogg_packet& packet);
};
