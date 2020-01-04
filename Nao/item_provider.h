#pragma once

#include "frameworks.h"

#include <cstdint>

class item_provider {
	public:
	item_provider() = delete;
	
	virtual ~item_provider() = 0;

	virtual LPWSTR name(size_t index) = 0;
	virtual int64_t size(size_t index) = 0;
	virtual LPWSTR type(size_t index) = 0;
	virtual double compression(size_t index) = 0;
};
