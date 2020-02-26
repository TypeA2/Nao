#include "vorbis_encoder.h"

vorbis_encoder::vorbis_encoder() {
    vorbis_info_init(&_vi);
    vorbis_comment_init(&_vc);
}

vorbis_encoder::~vorbis_encoder() {
    vorbis_info_clear(&_vi);
    vorbis_comment_clear(&_vc);
}

int vorbis_encoder::status() const {
    return _status;
}

bool vorbis_encoder::headerin(const ogg_packet& packet) {
    _status =  vorbis_synthesis_headerin(&_vi, &_vc, const_cast<ogg_packet*>(&packet));
    return _status == 0;
}

void vorbis_encoder::add_tag(const std::string& tag, const std::string& contents) {
    vorbis_comment_add_tag(&_vc, tag.c_str(), contents.c_str());
}


long vorbis_encoder::blocksize(const ogg_packet& packet) {
    return vorbis_packet_blocksize(&_vi, const_cast<ogg_packet*>(&packet));
}
