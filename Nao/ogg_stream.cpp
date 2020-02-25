#include "ogg_stream.h"

ogg_stream::ogg_stream(const ostream_ptr& out, int serialno) : _out { out } {
    ogg_stream_init(&_os, serialno);
}

ogg_stream::~ogg_stream() {
    flush();

    ogg_stream_clear(&_os);
}

ogg_packet ogg_stream::packet(char* data, int64_t size, bool eos) {
    ogg_packet packet {
        .packet = reinterpret_cast<unsigned char*>(data),
        .bytes = static_cast<long>(size),
        .b_o_s = _b_o_s ? 1 : 0,
        .e_o_s = eos ? 1 : 0,
        .granulepos = 0,
        .packetno = _current_packtet++
    };

    if (_b_o_s) {
        _b_o_s = false;
    }

    return packet;
}

void ogg_stream::packetin(const ogg_packet& packet) {
    ogg_stream_packetin(&_os, const_cast<ogg_packet*>(&packet));
}

void ogg_stream::pageout() {
    ogg_page page;
    while (ogg_stream_pageout(&_os, &page)) {
        _out->write(page.header, page.header_len);
        _out->write(page.body, page.body_len);
    }
}

void ogg_stream::flush() {
    ogg_page page;
    while (ogg_stream_flush(&_os, &page) != 0) {
        _out->write(page.header, page.header_len);
        _out->write(page.body, page.body_len);
    }
}
