#pragma once

#include "ui_element.h"

#include <string>

class line_edit : public ui_element {
    public:
    explicit line_edit(ui_element* parent);
    line_edit(ui_element* parent, const std::string& text);

    void set_read_only(bool readonly) const;
};

