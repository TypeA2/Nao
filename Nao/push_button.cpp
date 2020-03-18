#include "push_button.h"
#include "utils.h"

push_button::push_button(ui_element* parent) : ui_element(parent, WC_BUTTON, WS_CHILD | WS_VISIBLE | WS_TABSTOP) {
    set_font(win32::stock_object<HFONT>(DEFAULT_GUI_FONT));
}
push_button::push_button(ui_element* parent, const std::string& text) : push_button(parent) {
    set_text(text);
}

push_button::push_button(ui_element* parent, const win32::icon& icon) : push_button(parent) {
    set_style(BS_ICON, true);
    set_icon(icon);
}

push_button::push_button(ui_element* parent, const std::string& text, const win32::icon& icon) : push_button(parent) {
    set_text(text);
    set_icon(icon);
}

void push_button::set_text(const std::string& text) const {
    (void) send_message(WM_SETTEXT, 0, utils::utf16(text).c_str());
}

void push_button::set_icon(const win32::icon& icon) const {
    (void) send_message(BM_SETIMAGE, IMAGE_ICON, icon);
}
