#pragma once

#include "ui_element.h"

enum label_type {
    LABEL_SIMPLE = SS_SIMPLE,
    LABEL_LEFT = SS_LEFT,
    LABEL_LEFT_NO_WRAP = SS_LEFTNOWORDWRAP,
    LABEL_CENTER = SS_CENTER,
    LABEL_RIGHT = SS_RIGHT
};
    
class label : public ui_element {
    std::string _text;

    public:
    explicit label(ui_element* parent, const std::string& text = "", label_type type = LABEL_SIMPLE);

    // Use this label's text
    dimensions text_extent_point() const;

    void set_text(const std::string& text);
    const std::string& text() const;
};

