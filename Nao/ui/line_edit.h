#pragma once

#include "ui_element.h"
#include <string>

class data_model;

class line_edit : public ui_element {
    public:
    explicit line_edit(ui_element* parent);
    line_edit(ui_element* parent, const std::wstring& text);

    void set_text(const std::wstring& text) const;
    void set_font(HFONT font) const;

    private:
    void _init();
};

