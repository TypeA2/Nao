#include "main_window.h"

#include "resource.h"

#include "left_window.h"
#include "right_window.h"

#include "utils.h"
#include "dimensions.h"

#include "push_button.h"

#include <clocale>
#include <filesystem>
#include <logging.h>
#include <strings.h>

static rectangle start_rect() {
    return {
        .x = (GetSystemMetrics(SM_CXSCREEN) - dims::base_window_width) / 2,
        .y = (GetSystemMetrics(SM_CYSCREEN) - dims::base_window_height) / 2,
        .width = dims::base_window_width,
        .height = dims::base_window_height
    };
}

main_window::main_window(nao_view& view)
    : ui_element(nullptr, win32::wnd_class {
        .class_name = win32::load_wstring(IDC_NAO),
        .icon = win32::load_icon(IDI_NAO),
        .menu_resource = IDC_NAO
    }, start_rect(), WS_OVERLAPPEDWINDOW | WS_VISIBLE)
    , _view { view }
    , _left { this, _view }
    , _right { this } {
    
    set_window_text(win32::load_wstring(IDS_APP_TITLE));
}

left_window& main_window::left() {
    return _left;
}

right_window& main_window::right() {
    return _right;
}

void main_window::wm_destroy() {
    SDL_Event event;
    event.type = SDL_QUIT;
    event.quit.timestamp = SDL_GetTicks();
    SDL_PushEvent(&event);
}

void main_window::wm_size(int, const dimensions& dims) {
    int64_t window_width = (dims.width - dims::gutter_size) / 2;

    defer_window_pos()
        .move(_left, { 0, 0, window_width, dims.height })
        .move(_right, { window_width + dims::gutter_size, 0, window_width, dims.height });
}

static INT_PTR _about(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    // Simpler
    switch (msg) {
        case WM_INITDIALOG:
            return true;

        case WM_NOTIFY: {
            NMHDR* nm = reinterpret_cast<NMHDR*>(lparam);
            if (nm->idFrom == IDC_TPL_LINK && nm->code == NM_CLICK) {
                // License link clicked

                WCHAR path[MAX_PATH];
                DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);

                std::filesystem::path fs_path = std::filesystem::path(path).parent_path().string() + "\\license\\third-party";
                LPITEMIDLIST idl = ILCreateFromPathW(fs_path.c_str());

                // Find first in tree
                while (!idl && fs_path.string().length() >= len) {
                    fs_path = fs_path.parent_path();

                    idl = ILCreateFromPathW(fs_path.c_str());
                }

                if (idl) {
                    // I don't even know what the API is doing at this point
                    ShellExecuteA(hwnd, "open", fs_path.string().c_str(), nullptr, strings::to_utf8(path).c_str(), SW_SHOW);
                    ILFree(idl);
                }
            }
            break;
        }

        case WM_COMMAND:
            if (!(LOWORD(wparam) == IDOK || LOWORD(wparam) == IDCANCEL)) {
                break;
            }

        case WM_DESTROY:
            EndDialog(hwnd, LOWORD(wparam));
            return 0;

        default: break;
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);

};

void main_window::wm_command(WORD id, WORD code, HWND target) {
    switch (id) {
        case IDM_ABOUT: {
            DialogBoxW(win32::instance(), MAKEINTRESOURCEW(IDD_ABOUTBOX), handle(), _about);
            SetFocus(handle());
            break;
        }

        case IDM_EXIT:
            destroy();
            break;

        case ID_FILE_OPEN:
            _view.button_clicked(BUTTON_BROWSE);
            break;

        default:
            logging::coutln("WM_COMMAND", code);
            ui_element::wnd_proc(handle(), WM_COMMAND, MAKEWPARAM(id, code), reinterpret_cast<LPARAM>(target));
    }
}
