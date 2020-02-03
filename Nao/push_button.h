#pragma once

#include "ui_element.h"
#include "icon.h"

#include <string>

class push_button : public ui_element {
    protected:
    explicit push_button(ui_element* parent);

    public:
    explicit push_button(ui_element* parent, const icon& icon);
    explicit push_button(ui_element* parent, const std::string& text);
    explicit push_button(ui_element* parent, const std::string& text, const icon& icon);
    
    ~push_button() override = default;

    void set_text(const std::string& text) const;
    void set_icon(const icon& icon) const;
    void set_font(HFONT font) const;
};