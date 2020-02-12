#include "label.h"

#include "utils.h"

label::label(ui_element* parent, const std::string& text, label_type type) : ui_element(parent) {
    HWND handle = create_window(WC_STATICW, L"", WS_CHILD | WS_VISIBLE | type, { }, parent);

    ASSERT(handle);

    set_handle(handle);
    ui_element::set_font(HFONT(GetStockObject(DEFAULT_GUI_FONT)));

    set_text(text);
}

size label::text_extent_point(const std::string& str) const {
    if (str.empty()) {
        return ui_element::text_extent_point(_m_text);
    }

    return ui_element::text_extent_point(str);
}

void label::set_text(const std::string& text) {
    _m_text = text;
    SetWindowTextW(handle(), utils::utf16(text).c_str());
}

const std::string& label::get_text() const {
    return _m_text;
}
