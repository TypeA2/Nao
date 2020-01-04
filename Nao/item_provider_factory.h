#pragma once

#include "frameworks.h"

#include <vector>

class item_provider;

class item_provider_factory {
	public:
	using create_func = item_provider* (*)(HANDLE);

	static size_t register_class(create_func creator);
	static item_provider* create(size_t id, HANDLE file);

	private:
	static std::vector<create_func> _registered_classes;
	static size_t _next_id;
};

