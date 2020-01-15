#include "right_window.h"

#include "utils.h"
#include "resource.h"
#include "frameworks.h"
#include "dimensions.h"

right_window::right_window(ui_element* parent, data_model& model)
    : ui_element(parent)
    , _m_model(model) {
    ASSERT(parent);

    _init();
}

right_window::~right_window() {
    (void) this;
}



void right_window::wm_paint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(handle(), &ps);

    RECT rect;
    GetClientRect(handle(), &rect);

    HBRUSH brush = CreateSolidBrush(RGB(0xff, 0, 0));

    FillRect(hdc, &rect, brush);

    DeleteObject(brush);

    EndPaint(handle(), &ps);
}



void right_window::_init() {
    HINSTANCE inst = GetModuleHandleW(nullptr);
    union {
        LPCWSTR str;
        WCHAR buf[sizeof(str) / sizeof(WCHAR)];
    } pun { };

    int classname_length = LoadStringW(inst, IDS_RIGHT_WINDOW, pun.buf, 0);
    std::wstring class_name(classname_length + 1i64, L'\0');
    wcsncpy_s(class_name.data(), classname_length + 1i64, pun.str, classname_length);

    WNDCLASSEXW wcx {
        sizeof(wcx),
        CS_HREDRAW | CS_VREDRAW,
        wnd_proc_fwd,
        0, 0, inst,
        nullptr, LoadCursorW(nullptr, IDC_ARROW),
        HBRUSH(COLOR_WINDOW + 1),
        nullptr, class_name.c_str(), nullptr
    };

    ASSERT(RegisterClassExW(&wcx) != 0);

    RECT rect;
    GetClientRect(parent()->handle(), &rect);
    int window_width = (rect.right - dims::gutter_size) / 2;

    HWND handle = CreateWindowExW(0, class_name.c_str(), L"",
        WS_CHILD | WS_VISIBLE | SS_SUNKEN,
        window_width + dims::gutter_size, 0, window_width, rect.bottom,
        parent()->handle(), nullptr, inst,
        new wnd_init(this, &right_window::_wnd_proc));

    ASSERT(handle);
}

LRESULT right_window::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}
