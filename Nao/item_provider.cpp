#include "item_provider.h"

size_t item_provider::count() const {
    return 0;
}

item_data& item_provider::data(size_t index) {
    (void) index;

    static item_data none;
    none = item_data();

    return none;
}

const item_data& item_provider::data(size_t index) const {
    return const_cast<item_provider*>(this)->data(index);
}

const std::string& item_provider::get_name() const {
    return name;
}

const item_provider::file_stream& item_provider::get_stream() const {
    return file;
}

item_provider::preview_type item_provider::preview() const {
    return preview_type::PreviewNone;
}

ui_element* item_provider::preview_element(ui_element* parent) const {
    return nullptr;
}

item_provider::file_stream item_provider::stream() const {
    return file;
}

item_provider::item_provider(file_stream file, std::string name, data_model& model)
    : file { std::move(file) }
    , name { std::move(name) }
    , model { model } {

}
