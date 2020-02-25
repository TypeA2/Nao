#include "vorbis_encoder.h"

vorbis_encoder::vorbis_encoder() {
    vorbis_info_init(&_vi);
    vorbis_comment_init(&_vc);
}

vorbis_encoder::~vorbis_encoder() {
    vorbis_info_clear(&_vi);
    vorbis_comment_clear(&_vc);
}

bool vorbis_encoder::headerin(const ogg_packet& packet) {
    return vorbis_synthesis_headerin(&_vi, &_vc, const_cast<ogg_packet*>(&packet)) == 0;
}

long vorbis_encoder::blocksize(const ogg_packet& packet) {
    return vorbis_packet_blocksize(&_vi, const_cast<ogg_packet*>(&packet));
}
