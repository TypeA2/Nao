#include "right_window.h"

#include "frameworks.h"
#include "resource.h"

#include "utils.h"
#include "dimensions.h"

right_window::right_window(ui_element* parent, nao_view* view) : ui_element(parent) {
    std::wstring class_name = load_wstring(IDS_RIGHT_WINDOW);

    WNDCLASSEXW wcx {
        .cbSize        = sizeof(WNDCLASSEXW),
        .style         = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc   = wnd_proc_fwd,
        .hInstance     = instance(),
        .hCursor       = LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = HBRUSH(COLOR_WINDOW + 1),
        .lpszClassName = class_name.c_str()
    };

    ASSERT(RegisterClassExW(&wcx) != 0);

    auto [x, y, width, height] = parent->dimensions();

    int window_width = (width - dims::gutter_size) / 2;

    HWND handle = create_window(class_name, L"", WS_CHILD | WS_VISIBLE | SS_SUNKEN,
        { window_width + dims::gutter_size, 0, window_width, height }, parent,
        new wnd_init(this, &right_window::_wnd_proc, view));

    ASSERT(handle);
}

/*
void right_window::set_preview(const std::shared_ptr<ui_element>& element, preview_type type) {
    ASSERT(element);
    _m_preview = element;

    _m_type = type;
}

void right_window::clear_preview() {
    _m_preview.reset();
    _m_preview = nullptr;
    _m_type = preview_type::PreviewNone;
}

std::weak_ptr<ui_element> right_window::preview() const {
    return _m_preview;
}

right_window::preview_type right_window::type() const {
    return _m_type;
}*/



void right_window::wm_size(int type, int width, int height) {
    //if (_m_preview) {
        //HDWP dwp = BeginDeferWindowPos(1);

        //_m_preview->move_dwp(dwp, 0, 0, width, height);

        //EndDeferWindowPos(dwp);
    //}
}

LRESULT right_window::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    /*switch (_m_type) {
        case preview_type::PreviewListView:
            return _wnd_proc_list_view(hwnd, msg, wparam, lparam);

        default: break;
    }*/

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}
/*
LRESULT right_window::_wnd_proc_list_view(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    ASSERT(_m_type == preview_type::PreviewListView);

    switch (msg) {
        case WM_NOTIFY: {
            NMHDR* nm = LPNMHDR(lparam);

            if (nm->hwndFrom == _m_preview->handle()) {
                switch (nm->code) {
                    case LVN_COLUMNCLICK: {
                        _m_model.sort_preview(LPNMLISTVIEW(nm)->iSubItem);
                        break;
                    }

                    case NM_DBLCLK: {
                        _m_model.opened_preview(LPNMITEMACTIVATE(nm)->iItem);
                        break;
                    }

                    case NM_RCLICK: {
                        _m_model.context_menu_preview(LPNMITEMACTIVATE(nm)->ptAction);
                        break;
                    }
                }
            }
        }

        case WM_COMMAND: {
            HWND target = HWND(lparam);

            if (!target) {
                if (HIWORD(wparam) == 0) {
                    _m_model.menu_clicked(LOWORD(wparam));
                }
            }
        }

        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}
*/
