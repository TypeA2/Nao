#include "push_button.h"
#include "utils.h"

push_button::push_button(ui_element* parent) : ui_element(parent) {
    _init();
}

push_button::push_button(ui_element* parent, const std::wstring& text) : push_button(parent) {
    set_text(text);
}

push_button::push_button(ui_element* parent, HICON icon) : push_button(parent) {
    ui_element::set_style(BS_ICON, true);
    set_icon(icon);
}

push_button::push_button(ui_element* parent, const std::wstring& text, HICON icon)
    : push_button(parent) {
    set_text(text);
    set_icon(icon);
}


void push_button::set_text(const std::wstring& text) const {
    SendMessageW(handle(), WM_SETTEXT, 0, LPARAM(text.c_str()));
}

void push_button::set_icon(HICON icon) const {
    SendMessageW(handle(), BM_SETIMAGE, IMAGE_ICON, LPARAM(icon));
}

void push_button::set_font(HFONT font) const {
    SendMessageW(handle(), WM_SETFONT, WPARAM(font), 0);
}



void push_button::_init() {
    HWND handle = CreateWindowExW(0, WC_BUTTONW, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        0, 0, 0, 0,
        parent()->handle(), nullptr, GetModuleHandleW(nullptr), nullptr);

    ASSERT(handle);

    set_handle(handle);

    set_font(HFONT(GetStockObject(DEFAULT_GUI_FONT)));
}

