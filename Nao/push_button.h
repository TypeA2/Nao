#pragma once

#include "ui_element.h"

#include <string>

class push_button : public ui_element {
    // Internally construct by this
    explicit push_button(ui_element* parent);

    public:
    explicit push_button(ui_element* parent, const win32::icon& icon);
    explicit push_button(ui_element* parent, const std::string& text);
    explicit push_button(ui_element* parent, const std::string& text, const win32::icon& icon);
    
    ~push_button() override = default;

    void set_icon(const win32::icon& icon) const;
};
