#pragma once

#include "frameworks.h"

#include <cstdint>

class item_provider {
	public:
	virtual ~item_provider() { };

	virtual LPWSTR name(size_t index) = 0;
	virtual int64_t size(size_t index) = 0;
	virtual LPWSTR type(size_t index) = 0;
	virtual double compression(size_t index) = 0;
};

