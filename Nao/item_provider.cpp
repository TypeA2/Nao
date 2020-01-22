#include "item_provider.h"

item_provider::item_provider(stream file, std::string name, data_model& model)
    : file { std::move(file) }
    , name { std::move(name) }
    , model { model } {

}

const item_provider::item_data& item_provider::data(size_t index) const {
    return const_cast<item_provider*>(this)->data(index);
}

const std::string& item_provider::get_name() const {
    return name;
}
