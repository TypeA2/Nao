#pragma once

#include "file_handler.h"

class filesystem_handler : public item_file_handler {
    public:
    explicit filesystem_handler(const std::string& path);

    file_handler_tag tag() const override;
};
