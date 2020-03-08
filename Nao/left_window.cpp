#include "left_window.h"

#include "resource.h"

#include "utils.h"
#include "dimensions.h"

#include "list_view.h"
#include "line_edit.h"
#include "push_button.h"

#include "nao_view.h"

#include <string>

left_window::left_window(ui_element* parent, nao_view* view) : ui_element(parent), view(view) {
    std::wstring class_name = load_wstring(IDS_LEFT_WINDOW);

    // Register class
    WNDCLASSEXW wcx {
        .cbSize        = sizeof(wcx),
        .style         = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc   = wnd_proc_fwd,
        .hInstance     = instance(),
        .hCursor       = LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = HBRUSH(COLOR_WINDOW + 1),
        .lpszClassName = class_name.c_str()
    };

    ASSERT(RegisterClassExW(&wcx) != 0);

    auto [width, height] = parent->dims();
    int64_t window_width = (width - dims::gutter_size) / 2;

    HWND handle = create_window(class_name, L"", WS_CHILD | WS_VISIBLE | SS_SUNKEN,
        { .x = 0, .y = 0, .width = window_width, .height = height },
        parent, new wnd_init(this, &left_window::_wnd_proc));

    ASSERT(handle);
}

line_edit* left_window::path() const {
    return _m_path.get();
}

push_button* left_window::view_up() const {
    return _m_up.get();
}

list_view* left_window::list() const {
    return _m_list.get();
}

bool left_window::wm_create(CREATESTRUCTW* create) {
    _m_list = std::make_unique<list_view>(this, nao_view::list_view_header(), nao_view::shell_image_list());
    _m_list->set_column_alignment(2, list_view::Right);

    _m_path = std::make_unique<line_edit>(this);
    _m_path->set_read_only(true);

    // Button icons
    win32::dynamic_library shell32("shell32.dll");

    _m_up = std::make_unique<push_button>(this, shell32.load_icon_scaled(16817, 16, 16));

    _m_refresh = std::make_unique<push_button>(this, shell32.load_icon_scaled(16739, 16, 16));
    _m_refresh->set_enabled(false);

    _m_browse = std::make_unique<push_button>(this, "Browse...", shell32.load_icon_scaled(4, 16, 16));

    defer_window_pos()
        .move(_m_up, { dims::gutter_size, dims::gutter_size,
            dims::control_button_width, dims::control_height + 2 })
        .move(_m_refresh, { dims::control_button_width + 2 * dims::gutter_size, dims::gutter_size,
            dims::control_button_width, dims::control_height + 2 });

    return true;
}

void left_window::wm_size(int type, int width, int height) {
    (void) type;

    defer_window_pos()
        .move(_m_path, { dims::path_x_offset, dims::gutter_size + 1,
            width - dims::path_x_offset - dims::browse_button_width - dims::gutter_size * 2, dims::control_height })
        .move(_m_list, { 0, dims::control_height + (dims::gutter_size * 2),
            width, height - (dims::gutter_size * 2) - dims::control_height })
        .move(_m_browse, { width - dims::browse_button_width - dims::gutter_size, dims::gutter_size,
            dims::browse_button_width, dims::control_height + 2 });
}

LRESULT left_window::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_NOTIFY: {
            NMHDR* nm = reinterpret_cast<NMHDR*>(lparam);
            
            if (nm->hwndFrom == _m_list->handle()) {
                view->list_clicked(nm);
            }
            break;
        }

        case WM_COMMAND: {
            HWND target = reinterpret_cast<HWND>(lparam);

            view_button_type btn = BUTTON_NONE;

            if (target == _m_up->handle()) {
                btn = BUTTON_UP;
            } else if (target == _m_browse->handle()) {
                btn = BUTTON_BROWSE;
            }

            if (btn != BUTTON_NONE) {
                view->button_clicked(btn);
            }

            break;
        }

        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return EXIT_SUCCESS;
}

