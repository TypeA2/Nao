#pragma once

#include "item_provider_factory.h"

#include <string>
#include <memory>

struct item_data {
    std::string name;
    std::string type;

    int64_t size {};
    std::string size_str;

    double compression {};

    int icon {};

    bool dir {};
    bool drive {};

    char drive_letter {};

    item_provider_factory::stream stream;

    std::shared_ptr<void> data;
};
