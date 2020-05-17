#include "file_handler.h"

#include <logging.h>

file_handler::~file_handler() {
    logging::coutln("[FILE] Deleting for", path);
}

const istream_ptr& file_handler::get_stream() const {
    return stream;
}

const std::string& file_handler::get_path() const {
    return path;
}

file_handler::file_handler(const istream_ptr& stream, const std::string& path)
    : stream(stream), path(path) {
    logging::coutln("[FILE] Creating for", path);
}

size_t item_file_handler::count() const {
    return items.size();
}

item_data& item_file_handler::data(size_t index) {
    return items[index];
}

const std::vector<item_data>& item_file_handler::data() const {
    return items;
}

file_handler_tag operator|(file_handler_tag left, file_handler_tag right) noexcept {
    return static_cast<file_handler_tag>(static_cast<uintmax_t>(left) | static_cast<uintmax_t>(right));
}

file_handler_tag operator&(file_handler_tag left, file_handler_tag right) noexcept {
    return static_cast<file_handler_tag>(static_cast<uintmax_t>(left) & static_cast<uintmax_t>(right));
}
