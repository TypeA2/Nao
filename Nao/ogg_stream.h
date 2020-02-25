#pragma once

#include "binary_stream.h"

#include <ogg/ogg.h>

class ogg_stream {
    ogg_stream_state _os;
    ostream_ptr _out;
    int64_t _current_packtet { };

    bool _b_o_s = true;

    public:
    explicit ogg_stream(const ostream_ptr& out, int serialno = 0);
    ~ogg_stream();

    ogg_packet packet(char* data, int64_t size, bool eos = false);

    void packetin(const ogg_packet& packet);

    // Write any full pages that are pending
    void pageout();

    // Write all remaining pages
    void flush();
};
