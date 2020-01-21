#include "right_window.h"

#include "utils.h"
#include "resource.h"
#include "frameworks.h"
#include "dimensions.h"
#include "data_model.h"

right_window::right_window(ui_element* parent, data_model& model)
    : ui_element(parent)
    , _m_model(model)
    , _m_preview { } {
    ASSERT(parent);

    _init();

    _m_model.set_right(this);
}

right_window::~right_window() {
    delete _m_preview;
}



void right_window::set_preview(ui_element* element, preview_type type) {
    ASSERT(element);
    _m_preview = element;

    _m_type = type;
}

void right_window::clear_preview() {
    delete _m_preview;
    _m_preview = nullptr;
    _m_type = preview_type::PreviewNone;
}

ui_element* right_window::preview() const {
    return _m_preview;
}

right_window::preview_type right_window::type() const {
    return _m_type;
}



void right_window::wm_size(int type, int width, int height) {
    if (_m_preview) {
        HDWP dwp = BeginDeferWindowPos(1);

        _m_preview->move_dwp(dwp, 0, 0, width, height);

        EndDeferWindowPos(dwp);
    }
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
    switch (_m_type) {
        case preview_type::PreviewListView:
            return _wnd_proc_list_view(hwnd, msg, wparam, lparam);

        default: break;
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

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

