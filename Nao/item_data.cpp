#include "item_data.h"
#include "item_provider.h"

std::string item_data::path() const {
    if (!provider) {
        return drive ? std::string { drive_letter, ':', '\\' } : name;
    }

    return drive
        ? std::string { drive_letter, ':', '\\' }
        : provider->get_path() + name + (dir ? "\\" : "");
}
