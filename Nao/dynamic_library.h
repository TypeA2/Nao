#pragma once

#include "frameworks.h"

#include "icon.h"

#include <string>

class dynamic_library {
    public:
    explicit dynamic_library();
    explicit dynamic_library(const std::string& name);
    ~dynamic_library();

    icon load_icon_scaled(int resource, int width, int height) const;

    private:
    HMODULE _m_handle;
};

