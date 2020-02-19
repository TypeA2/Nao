#pragma once

#include "file_handler.h"

class flac_handler : public pcm_file_handler {
    public:
    using pcm_file_handler::pcm_file_handler;

    file_handler_tag tag() const override;

    pcm_provider_ptr make_provider() override;
};