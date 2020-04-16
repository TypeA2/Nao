#pragma once

#include "file_handler.h"


class ffmpeg_image_handler : public image_file_handler {
    using image_file_handler::image_file_handler;

    file_handler_tag tag() const override;

    image_provider_ptr make_provider() override;
};
