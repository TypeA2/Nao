#include "item_data.h"
#include "file_handler.h"

std::string item_data::path() const {
    if (!handler) {
        return drive ? std::string { drive_letter, ':', '\\' } : name;
    }

    return drive
        ? std::string { drive_letter, ':', '\\' }
        : handler->get_path() + name + (dir ? "\\" : "");
}
