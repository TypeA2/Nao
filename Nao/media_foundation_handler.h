#pragma once

#include "file_handler.h"

class media_foundation_handler : public av_file_handler {
    public:
    using av_file_handler::av_file_handler;

    file_handler_tag tag() const override;
};
