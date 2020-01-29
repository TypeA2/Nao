#pragma once

#include <string>
#include <memory>

class binary_istream;

struct item_data {
    using istream_type = std::shared_ptr<binary_istream>;

    std::string name;
    std::string type;

    uintmax_t size;
    std::string size_str;

    double compression;

    int icon;

    bool dir;
    bool drive;

    char drive_letter;
    istream_type stream;

    std::shared_ptr<void> data;
};
