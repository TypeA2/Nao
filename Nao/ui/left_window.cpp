#include "left_window.h"

#include "resource.h"

#include "utils.h"
#include "dimensions.h"

#include "main_window.h"
#include "list_view.h"
#include "line_edit.h"
#include "push_button.h"

#include <string>
#include <thread>

left_window::left_window(ui_element* parent, data_model& model)
    : ui_element(parent)
    , _m_model { model } {
    ASSERT(parent);
    
    _init();
}

left_window::~left_window() {
    delete _m_list;
    delete _m_up;
    delete _m_refresh;
    delete _m_browse;
    delete _m_path;
}



bool left_window::wm_create(CREATESTRUCTW* create) {
    (void) create;
    // ListView ImageList
    IImageList* imglist;
    if (FAILED(SHGetImageList(SHIL_SMALL, IID_PPV_ARGS(&imglist)))) {
        utils::coutln("failed to retrieve shell image list");
        return false;
    }

    _m_list = new list_view(this, data_model::listview_header(), imglist);
    _m_model.set_listview(_m_list);

    _m_path = new line_edit(this);
    _m_model.set_path_edit(_m_path);
    _m_path->set_style(WS_DISABLED);

    // Button icons
    HMODULE shell32 = LoadLibraryW(L"shell32.dll");
    ASSERT(shell32);
    
    HICON up_icon;
    LoadIconWithScaleDown(shell32, MAKEINTRESOURCEW(16817), 16, 16, &up_icon);
    _m_up = new push_button(this, up_icon);
    _m_up->move(dims::gutter_size, dims::gutter_size,
        dims::control_button_width, dims::control_height + 2);
    _m_up->set_style(WS_DISABLED);
    DeleteObject(up_icon);


    HICON refresh_icon;
    LoadIconWithScaleDown(shell32, MAKEINTRESOURCEW(16739), 16, 16, &refresh_icon);
    _m_refresh = new push_button(this, refresh_icon);
    _m_refresh->move(dims::control_button_width + 2 * dims::gutter_size, dims::gutter_size,
        dims::control_button_width, dims::control_height + 2);
    _m_refresh->set_style(WS_DISABLED);
    DeleteObject(refresh_icon);


    HICON folder_icon;
    LoadIconWithScaleDown(shell32, MAKEINTRESOURCEW(4), 16, 16, &folder_icon);
    _m_browse = new push_button(this, L"Browse...", folder_icon);
    _m_browse->set_style(WS_DISABLED);
    DeleteObject(folder_icon);

    FreeLibrary(shell32);

    return true;
}

void left_window::wm_size(int type, int width, int height) {
    HDWP dwp = BeginDeferWindowPos(3);

    _m_path->move_dwp(dwp, dims::path_x_offset, dims::gutter_size + 1,
        width - dims::path_x_offset
        - dims::browse_button_width - dims::gutter_size * 2, dims::control_height);

    _m_list->move_dwp(dwp,
        0, dims::control_height + (dims::gutter_size * 2),
        width, height - (dims::gutter_size * 2) - dims::control_height);
    
    _m_browse->move_dwp(dwp,
        width - dims::browse_button_width - dims::gutter_size, dims::gutter_size, 
        dims::browse_button_width, dims::control_height + 2);

    EndDeferWindowPos(dwp);
}




void left_window::_init() {
    HINSTANCE inst = GetModuleHandleW(nullptr);

    // Load left window classname
    union {
        LPCWSTR str;
        WCHAR buf[sizeof(str) / sizeof(WCHAR)];
    } pun { };
    
    int classname_length = LoadStringW(inst, IDS_LEFT_WINDOW, pun.buf, 0);
    std::wstring class_name(classname_length + 1i64, L'\0');
    wcsncpy_s(class_name.data(), classname_length + 1i64, pun.str, classname_length);

    // Register class
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
        0, 0, window_width, rect.bottom,
        parent()->handle(), nullptr, inst,
        new wnd_init(this, &left_window::_wnd_proc));

    ASSERT(handle);
}

void left_window::_open_folder() {
    IFileOpenDialog* dialog;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&dialog));

    if (FAILED(hr)) {
        utils::coutln("Failed to create FileOpenDialog");
        return;
    }

    FILEOPENDIALOGOPTIONS options;
    hr = dialog->GetOptions(&options);
    if (SUCCEEDED(hr)) {
        dialog->SetOptions(options | FOS_PICKFOLDERS);

        hr = dialog->Show(handle());
        if (SUCCEEDED(hr)) {
            IShellItem* item;
            hr = dialog->GetResult(&item);
            if (SUCCEEDED(hr)) {
                LPWSTR path;
                hr = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
                if (FAILED(hr)) {
                    utils::coutln("Failed to get path");
                } else {
                    _move_to(path);
                }

                item->Release();
            }
        }
    }

    dialog->Release();
}

void left_window::_move_to(std::wstring path) {
    // Only move to an existing path

    if (path.find(L'\\') != std::wstring::npos) {
        bool success = true;

        std::wstring next_path;

        while (!(GetFileAttributesW(path.data()) & FILE_ATTRIBUTE_DIRECTORY)) {
            next_path = path.substr(0, path.find_last_of(L'\\'));
            if (path == next_path) {
                success = false;
                break;
            }

            path = next_path;
        }

        if (success) {
            _m_current_path = path;
            _update_view();
        } else {
            MessageBoxW(handle(), L"Failed to find existing folder in path", L"Error",
                MB_OK | MB_ICONEXCLAMATION);
        }
    }
}

void left_window::_sort(NMLISTVIEW* view) const{
    _m_model.sort_list(view->iSubItem);
}


void left_window::_update_view() {
    //SendMessageW(_m_left_path, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(_m_path.data()));

    void(__cdecl * fwd)(void*) = [](void* args) {
        (void) CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        main_window* _this = reinterpret_cast<main_window*>(args);
        //_this->_get_provider();
        //_this->_fill_view();
        //_this->_list_sort(_this->_m_selected_col);

        CoUninitialize();
    };

    size_t low;
    size_t high;
    GetCurrentThreadStackLimits(&low, &high);

    _beginthread(fwd, unsigned(high - low), this);

}



LRESULT left_window::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_NOTIFY: {
            NMHDR* nm = reinterpret_cast<NMHDR*>(lparam);

            if (nm->hwndFrom == _m_list->handle()) {
                switch (nm->code) {
                    case LVN_COLUMNCLICK: {
                        _sort(reinterpret_cast<NMLISTVIEW*>(nm));
                        break;
                    }

                    default:
                        break;
                }
            }
            break;
        }

        case WM_COMMAND:
            if (HWND(lparam) == _m_browse->handle()) {
                _open_folder();
            }
            break;

        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return EXIT_SUCCESS;
}

