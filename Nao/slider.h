#pragma once

#include "ui_element.h"

class slider : public ui_element {
    public:
    explicit slider(ui_element* parent, int64_t min, int64_t max, bool vertical = false);

    void set_position(int64_t val)const ;
    int64_t get_position() const;
};

