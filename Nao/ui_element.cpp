#include "ui_element.h"

#include "utils.h"
#include <unordered_set>

ui_element::ui_element(ui_element* parent, const std::wstring& classname, DWORD style, DWORD ex_style) {
    _handle = win32::create_window_ex(classname, L"",
        style, { }, parent, ex_style);

    ASSERT(_handle);
}


ui_element::ui_element(ui_element* parent) : _parent { parent } {

}

ui_element::~ui_element() {
    destroy();
}

bool ui_element::destroy() {
    HWND handle = _handle;
    _handle = nullptr;
    _m_created = false;
    return DestroyWindow(handle);
}

ui_element* ui_element::parent() const {
    return _parent;
}

HWND ui_element::handle() const {
    return _handle;
}

HDC ui_element::device_context() const {
    return GetDC(_handle);
}

dimensions ui_element::text_extent_point(const std::string& str) const {
    std::wstring wide = utils::utf16(str);
    SIZE size;
    ASSERT(GetTextExtentPoint32W(device_context(), wide.c_str(), utils::narrow<uint32_t>(wide.size()), &size));

    return { .width = size.cx, .height = size.cy };
}

int64_t ui_element::width() const {
    RECT rect;
    bool res = GetWindowRect(handle(), &rect);
    ASSERT(res);

    return rect.right - rect.left;
}

int64_t ui_element::height() const {
    RECT rect;
    bool res = GetWindowRect(handle(), &rect);
    ASSERT(res);

    return rect.bottom - rect.top;
}

coordinates ui_element::coords() const {
    RECT rect;
    GetClientRect(handle(), &rect);

    return {
        .x = rect.left,
        .y = rect.top
    };
}

dimensions ui_element::dims() const {
    RECT rect;
    GetClientRect(handle(), &rect);

    return {
        .width = rect.right,
        .height = rect.bottom
    };
}

rectangle ui_element::rect() const {
    RECT rect;
    GetClientRect(handle(), &rect);

    return {
        .x = rect.left,
        .y = rect.top,
        .width = rect.right,
        .height = rect.bottom
    };
}

void ui_element::move(const rectangle& rect) {
    SetWindowPos(handle(), nullptr,
        utils::narrow<int>(rect.x), utils::narrow<int>(rect.y),
        utils::narrow<int>(rect.width), utils::narrow<int>(rect.height), 0);
}

HDWP& ui_element::move_dwp(HDWP& dwp, const rectangle& rect) {
    dwp = DeferWindowPos(dwp, handle(), nullptr,
        utils::narrow<int>(rect.x), utils::narrow<int>(rect.y),
        utils::narrow<int>(rect.width), utils::narrow<int>(rect.height), 0);

    return dwp;
}

void ui_element::set_style(DWORD style, bool enable) {
    DWORD old_style = static_cast<DWORD>(GetWindowLongPtrW(handle(), GWL_STYLE));

    // Don't re-apply style
    if (enable && !(old_style & style)) {
        SetWindowLongPtrW(handle(), GWL_STYLE, old_style | style);
    } else if (!enable && (old_style & style)) {
        SetWindowLongPtrW(handle(), GWL_STYLE, old_style & ~style);
    }
}

void ui_element::set_ex_style(DWORD style, bool enable) {
    DWORD old_ex_style = utils::narrow<DWORD>(GetWindowLongPtrW(handle(), GWL_EXSTYLE));

    if (enable && !(old_ex_style & style)) {
        SetWindowLongPtrW(handle(), GWL_EXSTYLE, old_ex_style | style);
    } else if (!enable && (old_ex_style & style)) {
        SetWindowLongPtrW(handle(), GWL_EXSTYLE, old_ex_style & ~style);
    }
}


void ui_element::set_font(HFONT font) const {
    send_message(WM_SETFONT, WPARAM(font), true);
}

void ui_element::set_enabled(bool enabled) {
    EnableWindow(handle(), enabled);
}

void ui_element::set_focus() const {
    SetFocus(handle());
}

void ui_element::activate() {
    SetActiveWindow(handle());
}

LRESULT ui_element::send_message(UINT msg, WPARAM wparam, LPARAM lparam) const {
    return SendMessageW(handle(), msg, wparam, lparam);
}

bool ui_element::post_message(UINT msg, WPARAM wparam, LPARAM lparam) const {
    return PostMessageW(handle(), msg, wparam, lparam);
}

void ui_element::set_handle(HWND handle) {
    ASSERT(!_handle && handle);

    _handle = handle;
}

bool ui_element::wm_create(CREATESTRUCTW* create) {
    if (_wnd_proc) {
        return _wnd_proc(handle(), WM_CREATE, 0, LPARAM(create)) == 0;
    }

    return DefWindowProcW(handle(), WM_CREATE, 0, LPARAM(create)) == 0;
}

void ui_element::wm_destroy() {
    DefWindowProcW(handle(), WM_DESTROY, 0, 0);
}

void ui_element::wm_size(int type, int width, int height) {
    if (_wnd_proc) {
        _wnd_proc(handle(), WM_SIZE, type, MAKELPARAM(width, height));
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
    if (_wnd_proc) {
        _wnd_proc(handle(), WM_COMMAND, wparam, lparam);
    } else {
        DefWindowProcW(handle(), WM_COMMAND, wparam, lparam);
    }
}

ui_element::wnd_init::wnd_init(ui_element* element, const wnd_proc_func& proc, void* replacement)
    : element(element), proc(proc), replacement(replacement){

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
        if (_this->_wnd_proc) {
            return _this->_wnd_proc(hwnd, msg, wparam, lparam);
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
                        element->_wnd_proc = init->proc;
                    }

                    element->_m_created = true;
                }

                create->lpCreateParams = init->replacement;

                delete init;

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
