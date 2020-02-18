#pragma once

#include "binary_stream.h"

class binary_istream;
class item_file_handler;

struct item_data {
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
    istream_ptr stream;

    std::shared_ptr<void> data;

    std::string path() const;
};
