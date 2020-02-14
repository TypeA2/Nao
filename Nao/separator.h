#pragma once

#include "ui_element.h"

enum separator_type {
    SEPARATOR_HORIZONTAL,
    SEPARATOR_VERTICAL
};

class separator : public ui_element {
    public:
    separator(ui_element* parent, separator_type type);
};
