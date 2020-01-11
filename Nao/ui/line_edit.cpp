#include "line_edit.h"

#include "utils.h"
#include "dimensions.h"

line_edit::line_edit(ui_element* parent) : ui_element(parent) {
    ASSERT(parent && parent->handle());
    _init();
}

line_edit::line_edit(ui_element* parent, const std::wstring& text) : line_edit(parent) {
    set_text(text);
}

void line_edit::set_text(const std::wstring& text) const{
    SendMessageW(handle(), WM_SETTEXT, 0, LPARAM(text.c_str()));
}

void line_edit::set_font(HFONT font) const {
    SendMessageW(handle(), WM_SETFONT, WPARAM(font), true);
}

void line_edit::_init() {
    HWND handle = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, L"",
        WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0,
        parent()->handle(), nullptr, GetModuleHandleW(nullptr), nullptr);

    ASSERT(handle);

    set_handle(handle);

    set_font(HFONT(GetStockObject(DEFAULT_GUI_FONT)));
}
