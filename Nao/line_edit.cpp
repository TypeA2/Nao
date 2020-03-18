#include "line_edit.h"

#include "utils.h"
#include "win32.h"

line_edit::line_edit(ui_element* parent)
    : ui_element(parent, WC_EDITW, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE) {

    set_font(win32::stock_object<HFONT>(DEFAULT_GUI_FONT));
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
