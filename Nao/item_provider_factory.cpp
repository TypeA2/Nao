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

class item_provider* item_provider_factory::create(std::istream& file, main_window* window) {
	item_provider* p;

	for (create_func f : _registered_classes) {
		p = f(file, window);

		if (p) {
			return p;
		}
	}

	return nullptr;
}


item_provider* item_provider_factory::create(size_t id, std::istream& file, main_window* window) {
	return _registered_classes[id](file, window);
}
