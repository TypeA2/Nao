#include "data_model.h"

data_model::data_model(std::wstring initial_path)
	: _m_path { std::move(initial_path) } {
	
}

