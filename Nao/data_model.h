#pragma once

#include <string>

class data_model {
	public:
	data_model() = delete;
	explicit data_model(std::wstring initial_path);

	private:

	std::wstring _m_path;
};

