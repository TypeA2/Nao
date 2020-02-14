#include "line_edit.h"

#include "utils.h"

line_edit::line_edit(ui_element* parent) : ui_element(parent) {
    HWND handle = create_window_ex(WC_EDITW, L"", WS_CHILD | WS_VISIBLE,
        { }, parent, WS_EX_CLIENTEDGE);

    ASSERT(handle);

    set_handle(handle);

    ui_element::set_font(HFONT(GetStockObject(DEFAULT_GUI_FONT)));
}

line_edit::line_edit(ui_element* parent, const std::string& text) : line_edit(parent) {
    set_text(text);
}

void line_edit::set_text(const std::string& text) const {
    (void) send_message(WM_SETTEXT, 0, utils::utf16(text).c_str());
}

void line_edit::set_read_only(bool readonly) const {
    (void) send_message(EM_SETREADONLY, readonly);
}
