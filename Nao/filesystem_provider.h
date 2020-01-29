#pragma once

#include "item_provider.h"

class filesystem_provider : public item_provider {
    public:
    explicit filesystem_provider(const std::string& path);

    private:
    static item_provider_ptr _create(const istream_type& stream,
        const std::string& path);
    static size_t _id;
};
