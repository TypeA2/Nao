#include "right_window.h"

#include "frameworks.h"
#include "resource.h"

#include "utils.h"
#include "dimensions.h"
#include "list_view.h"

right_window::right_window(ui_element* parent, nao_view* view) : ui_element(parent) {
    std::wstring class_name = win32::load_wstring(IDS_RIGHT_WINDOW);

    WNDCLASSEXW wcx {
        .cbSize        = sizeof(WNDCLASSEXW),
        .style         = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc   = wnd_proc_fwd,
        .hInstance     = win32::instance(),
        .hCursor       = LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = HBRUSH(COLOR_WINDOW + 1),
        .lpszClassName = class_name.c_str()
    };

    ASSERT(RegisterClassExW(&wcx) != 0);

    auto [width, height] = parent->dims();

    int64_t window_width = (width - dims::gutter_size) / 2;

    HWND handle = win32::create_window(class_name, L"", WS_CHILD | WS_VISIBLE | SS_SUNKEN,
        { window_width + dims::gutter_size, 0, window_width, height }, parent, this);

    ASSERT(handle);
}

void right_window::set_preview(preview_ptr instance) {
    _m_preview = std::move(instance);
}

void right_window::remove_preview() {
    _m_preview.reset();
}

preview* right_window::get_preview() const {
    return _m_preview.get();
}

void right_window::wm_size(int type, int width, int height) {
    (void) type;

    if (_m_preview) {
        defer_window_pos()
            .move(_m_preview, { 0, 0, width, height });
    }
}
