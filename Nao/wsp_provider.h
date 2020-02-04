#pragma once

#include "item_provider.h"

class wsp_provider : public item_provider {
    public:
    wsp_provider(const istream_type& stream, const std::string& path);

    private:

    struct wwriff_file {
        uintmax_t size;
        uintmax_t offset;
    };

    std::vector<wwriff_file> _m_riff;
};
