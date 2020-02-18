#pragma once

#include <string>
#include <memory>

class binary_istream;
class item_file_handler;

struct item_data {
    using istream_type = std::shared_ptr<binary_istream>;

    item_file_handler* handler;

    std::string name;
    std::string type;

    std::streamsize size;
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
