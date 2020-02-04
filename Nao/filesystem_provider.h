#pragma once

#include "item_provider.h"

class filesystem_provider : public item_provider {
    public:
    explicit filesystem_provider(const std::string& path);
};
