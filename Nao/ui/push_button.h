#pragma once

#include "ui_element.h"

#include <string>

class push_button : public ui_element {
    public:
    explicit push_button(ui_element* parent);
    push_button(ui_element* parent, const std::wstring& text);

    void set_text(const std::wstring& text) const;
    void set_icon(HICON icon) const;
    void set_font(HFONT font) const;

    private:
    void _init();
};
