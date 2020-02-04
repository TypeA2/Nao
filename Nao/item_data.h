#pragma once

#include <string>
#include <memory>

class binary_istream;
class item_provider;

struct item_data {
    using istream_type = std::shared_ptr<binary_istream>;

    item_provider* provider;

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

    std::string path() const;
};
