#include "item_data.h"
#include "file_handler.h"

std::string item_data::path() const {
    if (!handler) {
        return drive ? std::string { drive_letter, ':', '\\' } : name;
    }

    std::string parent = handler->get_path();
    if (parent.back() != '\\') {
        parent.push_back('\\');
    }

    return drive
        ? std::string { drive_letter, ':', '\\' }
        : parent + name + (dir ? "\\" : "");
}
