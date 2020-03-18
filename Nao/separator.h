#pragma once

#include "ui_element.h"

enum separator_type : DWORD {
    SEPARATOR_HORIZONTAL = SS_ETCHEDHORZ,
    SEPARATOR_VERTICAL   = SS_ETCHEDVERT
};

class separator : public ui_element {
    public:
    separator(ui_element* parent, separator_type type);
};
