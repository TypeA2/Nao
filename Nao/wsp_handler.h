#pragma once

#include "file_handler.h"

class wsp_handler : public item_file_handler {
    public:
    wsp_handler(const istream_ptr& stream, const std::string& path);

    file_handler_tag tag() const override;

    private:
    struct wwriff_file {
        std::streamsize size;
        std::streamoff offset;
    };

    std::vector<wwriff_file> _m_riff;
};
