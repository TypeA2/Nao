#include "item_provider_factory.h"

std::vector<item_provider_factory::create_func>
    item_provider_factory::_registered_classes;

size_t item_provider_factory::_next_id = 0;

size_t item_provider_factory::register_class(create_func creator) {
    if (std::find(_registered_classes.begin(), 
        _registered_classes.end(), creator) != _registered_classes.end()) {
        return -1;
    }

    _registered_classes.push_back(creator);

    return _next_id++;
}

item_provider* item_provider_factory::create(const stream& file,
    const std::string& name, data_model& model) {
    item_provider* p;

    for (create_func f : _registered_classes) {
        p = f(file, name, model);

        if (p) {
            return p;
        }
    }

    return nullptr;
}


item_provider* item_provider_factory::create(size_t id, const stream& file,
    const std::string& name, data_model& model) {

    return _registered_classes[id](file, name, model);
}
