#include "separator.h"
#include "utils.h"

separator::separator(ui_element* parent, separator_type type) : ui_element(parent) {
    DWORD type_style = 0;

    switch (type) {
        case SEPARATOR_HORIZONTAL: type_style = SS_ETCHEDHORZ; break;
        case SEPARATOR_VERTICAL: type_style = SS_ETCHEDVERT; break;
    }

    HWND handle = win32::create_window(WC_STATICW, L"",
        WS_CHILD | WS_VISIBLE | type_style, { }, parent);

    ASSERT(handle);

    set_handle(handle);
}
