#include "ui_element.h"

#include "utils.h"

#include <cassert>
#include <functional>

ui_element::ui_element(ui_element* parent)
    : _m_parent { parent }
    , _m_handle { }
    , _m_mem_wnd_proc { }
    , _m_created { } {

}

ui_element::~ui_element() {
    destroy();
}

bool ui_element::destroy() {
    HWND handle = _m_handle;
    _m_handle = nullptr;
    return DestroyWindow(handle);
}

ui_element* ui_element::parent() const {
    return _m_parent;
}

HWND ui_element::handle() const {
    return _m_handle;
}

long ui_element::width() const {
    RECT rect;
    bool res = GetWindowRect(handle(), &rect);
    assert(res);

    return rect.right - rect.left;
}

long ui_element::height() const {
    RECT rect;
    bool res = GetWindowRect(handle(), &rect);
    assert(res);

    return rect.bottom - rect.top;
}

void ui_element::move(int x, int y, int width, int height) {
    SetWindowPos(handle(), nullptr, x, y, width, height, 0);
}

HDWP& ui_element::move_dwp(HDWP& dwp, int x, int y, int width, int height) {
    dwp = DeferWindowPos(dwp, handle(), nullptr, x, y, width, height, 0);

    return dwp;
}

void ui_element::set_style(DWORD style, bool enable) {
    LONG_PTR old_style = GetWindowLongPtrW(handle(), GWL_STYLE);
    
    if (enable) {
        SetWindowLongPtrW(handle(), GWL_STYLE, old_style | style);
    } else {
        SetWindowLongPtrW(handle(), GWL_STYLE, old_style & ~LONG_PTR(style));
    }
}


void ui_element::set_handle(HWND handle) {
    assert(!_m_handle && handle);

    _m_handle = handle;
}



bool ui_element::wm_create(CREATESTRUCTW* create) {
    if (_m_mem_wnd_proc) {
        return (this->*_m_mem_wnd_proc)(handle(), WM_CREATE, 0, LPARAM(create));
    }

    return DefWindowProcW(handle(), WM_CREATE, 0, LPARAM(create));
}

void ui_element::wm_destroy() {
    DefWindowProcW(handle(), WM_DESTROY, 0, 0);
}

void ui_element::wm_size(int type, int width, int height) {
    if (_m_mem_wnd_proc) {
        (this->*_m_mem_wnd_proc)(handle(), WM_SIZE, type, MAKELPARAM(width, height));
    } else {
        DefWindowProcW(handle(), WM_SIZE, type, MAKELPARAM(width, height));
    }
}

void ui_element::wm_paint() {
    // API dislikes no painting being done
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(handle(), &ps);
    (void) hdc;
    EndPaint(handle(), &ps);
}

void ui_element::wm_command(WPARAM wparam, LPARAM lparam) {
    if (_m_mem_wnd_proc) {
        (this->*_m_mem_wnd_proc)(handle(), WM_COMMAND, wparam, lparam);
    } else {
        DefWindowProcW(handle(), WM_COMMAND, wparam, lparam);
    }
}




ui_element::wnd_init::wnd_init(ui_element* element, member_wnd_proc<> proc) {
    this->element = element;
    this->proc = proc;
}

void ui_element::use_wnd_proc(member_wnd_proc<> new_proc) {
    assert(new_proc);
    _m_mem_wnd_proc = new_proc;
}

LRESULT ui_element::wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    // Retrieve instance
    ui_element* _this = reinterpret_cast<ui_element*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (_this) {
        // Predefined handlers
        switch (msg) {
            case WM_DESTROY:
                _this->wm_destroy();
                return 0;
            
            case WM_SIZE:
                _this->wm_size(int(wparam), LOWORD(lparam), HIWORD(lparam));
                return 0;

            case WM_PAINT:
                _this->wm_paint();
                return 0;

            case WM_COMMAND:
                _this->wm_command(wparam, lparam);
                return 0;

            default: break;
        }
        
        // Custom callback is set
        if (_this->_m_mem_wnd_proc) {
            return (_this->*(_this->_m_mem_wnd_proc))(hwnd, msg, wparam, lparam);
        }
    }

    // Some default handling if the instance is not yet set or not yet completed
    if (!_this || !_this->_m_created) {
        if (msg == WM_CREATE) {
            CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
            wnd_init* init = static_cast<wnd_init*>(create->lpCreateParams);

            // Initialisation given
            if (init) {
                ui_element* element = init->element;
                if (element) {
                    // Set handle and `this` pointer
                    element->set_handle(hwnd);
                    SetWindowLongPtrW(hwnd, GWLP_USERDATA, LONG_PTR(element));

                    // Set custom callback
                    if (init->proc) {
                        element->use_wnd_proc(init->proc);
                    }

                    element->_m_created = true;
                }

                delete init;

                create->lpCreateParams = nullptr;

                if (element) {
                    // Should automatically forward
                    return element->wm_create(create) ? 0 : -1;
                }

                // Call default
            }
        }
    }
    
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}
