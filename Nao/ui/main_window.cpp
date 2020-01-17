#include "main_window.h"

#include "resource.h"

#include "utils.h"
#include "item_provider.h"
#include "item_provider_factory.h"

#include "left_window.h"
#include "right_window.h"
#include "data_model.h"
#include "dimensions.h"

#include <process.h>

#include <clocale>
#include <algorithm>

main_window::main_window(HINSTANCE inst, int show_cmd, data_model& model)
    : ui_element(nullptr)
    , _m_success(false)
    , _m_inst(inst)

    , _m_left { }, _m_right { }
    
    , _m_accel { }
    , _m_model { model } {

    _m_inst = inst;
    ASSERT(inst);

    // Whether the setup was a success
    _m_success = _init(show_cmd);

    // We're done
    _m_model.set_window(this);
}

main_window::~main_window() {
    delete _m_left;
    delete _m_right;
}

int main_window::run() const {
    // Exit on failure
    if (!_m_success) {
        return EXIT_FAILURE;
    }

    // Process messages
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        if (!TranslateAcceleratorW(msg.hwnd, _m_accel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return int(msg.wParam);
}



bool main_window::wm_create(CREATESTRUCTW* create) {
    (void) create;
    _m_left = new left_window(this, _m_model);
    _m_right = new right_window(this, _m_model);

    return true;
}

void main_window::wm_destroy() {
    PostQuitMessage(EXIT_SUCCESS);
}

void main_window::wm_size(int type, int width, int height) {
    if (!_m_success) {
        // Members could still be null
        return;
    }
    
    int window_width = (width - dims::gutter_size) / 2;

    HDWP dwp = BeginDeferWindowPos(2);

    _m_left->move_dwp(dwp, 0, 0, window_width, height);
    _m_right->move_dwp(dwp, window_width + dims::gutter_size, 0, window_width, height);

    EndDeferWindowPos(dwp);
}

void main_window::wm_command(WPARAM wparam, LPARAM lparam) {
    switch (LOWORD(wparam)) {
        case IDM_ABOUT:
            DialogBoxW(_m_inst, MAKEINTRESOURCEW(IDD_ABOUTBOX), handle(), _about);
            break;

        case IDM_EXIT:
            DestroyWindow(handle());
            break;

        case ID_FILE_OPEN:
            //_open_folder();
            break;

        default:
            utils::coutln("WM_COMMAND", LOWORD(wparam));
            DefWindowProcW(handle(), WM_COMMAND, wparam, lparam);
    }
}


bool main_window::_init(int show_cmd) {
    // CRT locale
    std::setlocale(LC_ALL, "en_US.utf8");

    // Load strings
    std::wstring app_title;
    {
        union {
            LPCWSTR str;
            WCHAR buf[sizeof(str) / sizeof(WCHAR)];
        } pun { };
        
        for (const int& resource : { IDS_APP_TITLE, IDC_NAO }) {
            std::wstring* container;
            switch (resource) {
                case IDS_APP_TITLE:    container = &app_title;       break;
                case IDC_NAO:          container = &_m_window_class; break;
                default: continue;
            }

            int length = LoadStringW(_m_inst, resource, pun.buf, 0) + 1;
            *container = std::wstring(length, '\0');
            wcsncpy_s(container->data(), length, pun.str, length - 1i64);
        }
    }
    
    // Additional setup
    if (!_register_class() || !_init_instance(show_cmd, app_title)) {
        utils::coutln("_register_class or _init_instance failed");
        return false;
    }

    // Load accelerators
    _m_accel = LoadAcceleratorsW(_m_inst, MAKEINTRESOURCEW(IDC_NAO));
    ASSERT(_m_accel);
    
    return true;
}

bool main_window::_register_class() const {
    // Our window class instance
    WNDCLASSEXW wcex {
        sizeof(WNDCLASSEXW),
        CS_HREDRAW | CS_VREDRAW,
        // Setting the WndProc to this uses the default WndProc for everything
        wnd_proc_fwd,
        0,
        0,
        _m_inst,
        LoadIconW(_m_inst, MAKEINTRESOURCEW(IDI_NAO)),
        LoadCursorW(nullptr, IDC_ARROW),
        HBRUSH(COLOR_WINDOW + 1),
        MAKEINTRESOURCEW(IDC_NAO),
        _m_window_class.c_str(),
        LoadIconW(_m_inst, MAKEINTRESOURCEW(IDI_NAO))
    };

    if (RegisterClassExW(&wcex) == 0) {
        return false;
    }

    return true;
}

bool main_window::_init_instance(int show_cmd, const std::wstring& title) {
    (void) show_cmd;

    // Centering
    int nx_size = GetSystemMetrics(SM_CXSCREEN);
    int ny_size = GetSystemMetrics(SM_CYSCREEN);

    int nx_pos = (nx_size - 1280) / 2;
    int ny_pos = (ny_size - 960) / 2;

    // Create the main window
    HANDLE hwnd = CreateWindowExW(0, _m_window_class.c_str(), title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        nx_pos, ny_pos, 1280, 800,
        nullptr, nullptr, _m_inst,
        new wnd_init(this, &main_window::_wnd_proc));

    if (!hwnd) {
        utils::coutln("main window creation failed");
        return false;
    }

    return true;
}


LRESULT main_window::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    (void) this;
    
    // Message processing
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

INT_PTR main_window::_about(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    (void) lparam;

    // Simpler
    switch (msg) {
        case WM_INITDIALOG:
            return 1;

        case WM_COMMAND:
            if (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDCANCEL) {
                EndDialog(hwnd, LOWORD(wparam));
                return 1;
            }

        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}
