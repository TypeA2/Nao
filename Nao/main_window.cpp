#include "main_window.h"

#include "resource.h"

#include "left_window.h"
#include "right_window.h"

#include "utils.h"
#include "dimensions.h"

#include <clocale>

main_window::main_window(nao_view* view) : ui_element(nullptr) {
    // CRT locale
    std::setlocale(LC_ALL, "en_US.utf8");

    std::wstring window_class = load_wstring(IDC_NAO);
    // Our window class instance
    WNDCLASSEXW wcex {
        .cbSize        = sizeof(WNDCLASSEXW),
        .style         = CS_HREDRAW | CS_VREDRAW,
        // Setting the WndProc to this uses the default WndProc for everything
        .lpfnWndProc   = wnd_proc_fwd,
        .hInstance     = instance(),
        .hIcon         = load_icon(IDI_NAO),
        .hCursor       = LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = HBRUSH(COLOR_WINDOW + 1),
        .lpszMenuName  = MAKEINTRESOURCEW(IDC_NAO),
        .lpszClassName = window_class.c_str(),
        .hIconSm       = load_icon(IDI_NAO)
    };

    ASSERT(RegisterClassExW(&wcex) != 0);

    // Centering
    int nx_size = GetSystemMetrics(SM_CXSCREEN);
    int ny_size = GetSystemMetrics(SM_CYSCREEN);

    int nx_pos = (nx_size - dims::base_window_width) / 2;
    int ny_pos = (ny_size - dims::base_window_height) / 2;

    // Create the main window

    HANDLE hwnd = create_window(
        window_class, load_wstring(IDS_APP_TITLE), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        { nx_pos, ny_pos, dims::base_window_width, dims::base_window_height },
        nullptr, new wnd_init(this, &main_window::_wnd_proc, view)
    );

    ASSERT(hwnd);
}

left_window* main_window::left() const {
    return _m_left.get();
}

right_window* main_window::right() const {
    return _m_right.get();
}

bool main_window::wm_create(CREATESTRUCTW* create) {
    _m_left = std::make_unique<left_window>(this, static_cast<nao_view*>(create->lpCreateParams));
    _m_right = std::make_unique<right_window>(this, static_cast<nao_view*>(create->lpCreateParams));

    return true;
}

void main_window::wm_destroy() {
    PostQuitMessage(EXIT_SUCCESS);
}

void main_window::wm_size(int type, int width, int height) {
    (void) type;
    int window_width = (width - dims::gutter_size) / 2;

    defer_window_pos<2>()
        .move(_m_left, { 0, 0, window_width, height })
        .move(_m_right, { window_width + dims::gutter_size, 0, window_width, height });
}

void main_window::wm_command(WPARAM wparam, LPARAM lparam) {
    switch (LOWORD(wparam)) {
        case IDM_ABOUT: {
            static DLGPROC _about = 
                [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> INT_PTR {
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
                };

            DialogBoxW(instance(), MAKEINTRESOURCEW(IDD_ABOUTBOX), handle(), _about);
            break;
        }

        case IDM_EXIT:
            destroy();
            break;

        case ID_FILE_OPEN:
            //_open_folder();
            break;

        default:
            utils::coutln("WM_COMMAND", LOWORD(wparam));
            DefWindowProcW(handle(), WM_COMMAND, wparam, lparam);
    }
}

LRESULT main_window::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    (void) this;

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}
