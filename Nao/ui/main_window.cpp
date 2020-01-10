#include "main_window.h"

#include "resource.h"

#include "utils.h"
#include "item_provider.h"
#include "item_provider_factory.h"

#include "left_window.h"

#include <process.h>

#include <clocale>
#include <algorithm>
#include <cassert>

main_window::main_window(HINSTANCE inst, int show_cmd, data_model& model)
    : ui_element(nullptr)
    , _m_success(false)
    , _m_inst(inst)

    , _m_hwnd {}	  , _m_left {}		  , _m_right {}
    
    , _m_accel { }
    , _m_model { model } {
    
    _m_inst = inst;
    assert(inst);

    // Whether the setup was a success
    _m_success = _init(show_cmd);
}

main_window::~main_window() {
    delete _m_left;
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
    if (!_create_subwindows()) {
        utils::coutln("_create_subwindows failed");
        return false;
    }

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
    
    int window_width = (width - gutter_size) / 2;

    HDWP dwp = BeginDeferWindowPos(2);
    _m_left->move_dwp(dwp, 0, 0, window_width, height);
    dwp = DeferWindowPos(dwp, _m_right, nullptr,
        window_width + gutter_size, 0, window_width, height, 0);

    EndDeferWindowPos(dwp);
}



bool main_window::_init(int show_cmd) {
    // CRT locale
    std::setlocale(LC_ALL, "en_US.utf8");

    std::wstring app_title;
    {
        union {
            LPCWSTR str;
            WCHAR buf[sizeof(str) / sizeof(WCHAR)];
        } pun { };
    	
        for (const int& resource : { IDS_APP_TITLE, IDC_NAO, IDS_RIGHT_WINDOW }) {
            std::wstring* container;
            switch (resource) {
                case IDS_APP_TITLE:	   container = &app_title;		 break;
                case IDC_NAO:		   container = &_m_window_class; break;
                case IDS_RIGHT_WINDOW: container = &_m_right_class;  break;
                default: continue;
            }

            int length = LoadStringW(_m_inst, resource, pun.buf, 0);
            *container = std::wstring(length + 1i64, '\0');
            wcsncpy_s(container->data(), length + 1i64, pun.str, length);
        }
    }
    
    // Load string resources
    //LoadStringW(_m_inst, IDS_APP_TITLE, _m_title, STRING_SIZE);
    //LoadStringW(_m_inst, IDC_NAO, _m_window_class, STRING_SIZE);
    //LoadStringW(_m_inst, IDS_LEFT_WINDOW, _m_left_window, STRING_SIZE);
    //LoadStringW(_m_inst, IDS_RIGHT_WINDOW, _m_right_window, STRING_SIZE);

    // Additional setup
    if (!_register_class() || !_init_instance(show_cmd, app_title)) {
        utils::coutln("_register_class or _init_instance failed");
        return false;
    }

    // Load accelerators
    _m_accel = LoadAcceleratorsW(_m_inst, MAKEINTRESOURCEW(IDC_NAO));
    assert(_m_accel);
    
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

    // Left window class

    //wcex.lpfnWndProc = _left_proc_fwd;
    //wcex.lpszClassName = _m_left_window;

    //if (RegisterClassExW(&wcex) == 0) {
    //	return false;
    //}

    // Right window
    wcex.lpfnWndProc = _right_proc_fwd;
    wcex.lpszClassName = _m_right_class.c_str();

    return RegisterClassExW(&wcex) != 0;
}

bool main_window::_init_instance(int show_cmd, const std::wstring& title) {
    (void) show_cmd;
    int nx_size = GetSystemMetrics(SM_CXSCREEN);
    int ny_size = GetSystemMetrics(SM_CYSCREEN);

    int nx_pos = (nx_size - 1280) / 2;
    int ny_pos = (ny_size - 960) / 2;

    // Create the main window
    _m_hwnd = CreateWindowExW(0, _m_window_class.c_str(), title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        nx_pos, ny_pos, 1280, 800,
        nullptr, nullptr, _m_inst,
        new wnd_init(this, &main_window::_wnd_proc));

    if (!_m_hwnd) {
        utils::coutln("main window creation failed");
        return false;
    }

    return true;
}

bool main_window::_create_subwindows() {
    RECT rect;
    GetClientRect(_m_hwnd, &rect);
    
    int window_width = (rect.right - gutter_size) / 2;

    _m_left = new left_window(this);

    _m_right = CreateWindowExW(0, _m_right_class.c_str(), L"",
        WS_CHILD | WS_VISIBLE | SS_SUNKEN,
        window_width + gutter_size, 0,
        window_width, rect.bottom,
        handle(), nullptr, _m_inst, nullptr);

    if (!_m_right) {
        utils::coutln("creating right window failed", GetLastError());
        return false;
    }

    return true;
}

LRESULT main_window::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    // Message processing
    switch (msg) {
        case WM_COMMAND: {
            switch (LOWORD(wparam)) {
                case IDM_ABOUT:
                    DialogBoxW(_m_inst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hwnd, _about);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    break;
                case ID_FILE_OPEN:
                    //_open_folder();
                    break;
                default:
                    utils::coutln("WM_COMMAND", LOWORD(wparam));
                    return DefWindowProcW(hwnd, msg, wparam, lparam);
            }
            break;
        }
        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return EXIT_SUCCESS;
}

void main_window::_list_sort(int col) {
    /*for (int i = 0; i < int(std::size(_m_sort_order)); ++i) {
        if (i == col) {
            _set_left_sort_arrow(i,
                (_m_sort_order[i] == REVERSE) ? DOWN_ARROW : UP_ARROW);
        } else {
            _set_left_sort_arrow(i, NO_ARROW);
        }
    }

    utils::coutln("sorting");

    int (CALLBACK * comp)(LPARAM, LPARAM, LPARAM) =
        [](LPARAM lparam1, LPARAM lparam2, LPARAM info) -> int {

        list_item_data* item1 = reinterpret_cast<list_item_data*>(lparam1);
        list_item_data* item2 = reinterpret_cast<list_item_data*>(lparam2);

        if (!item1 || !item2) {
            return 0;
        }

        sort_order order = static_cast<sort_order>(HIWORD(info) & 0xFF);

        int first1 = 0;
        int first2 = 0;

        switch (order) {
            case NORMAL:
                first1 = -1;
                first2 = 1;
                break;

            case REVERSE:
                first1 = 1;
                first2 = -1;
                break;

            default:
                break;
        }

        // Directories on top
        if (!item1->dir != !item2->dir) {
            return item1->dir ? -1 : 1;
        }

        const auto& f = std::use_facet<std::ctype<std::wstring::value_type>>(std::locale());

        auto cmp = [&](const std::wstring& left, const std::wstring& right) -> int {
            return std::lexicographical_compare(
                left.begin(), left.end(), right.begin(), right.end(),
                [&f](std::wstring::value_type a, std::wstring::value_type b) {
                    return f.tolower(a) < f.tolower(b);
                }) ? first1 : first2;
        };

        switch (LOWORD(info)) {
            case 0: // Name, alphabetically
                if (item1->name == item2->name) { return 0; }

                return cmp(item1->name, item2->name);
            case 1: { // Type, alphabetically
                if (item1->type == item2->type) {
                    // Fallback on name
                    return cmp(item1->name, item2->name);
                }

                return cmp(item1->type, item2->type); }

            case 2: // File size
                if (item1->size == item2->size) {
                    // Fallback on name
                    return cmp(item1->name, item2->name);
                }

                return (item1->size < item2->size) ? first1 : first2;

            case 3: // Compression ratio
                if (item1->compression == item2->compression) {
                    // Fallback on name
                    return cmp(item1->name, item2->name);
                }

                return (item1->compression < item2->compression) ? first1 : first2;

            default: return 0;
        }
    };

    ListView_SortItems(_m_left_list->handle(), comp, MAKELPARAM(col, _m_sort_order[col]));*/
}


/*
void main_window::_get_provider() {
    item_provider* p = nullptr;
    if (GetFileAttributesW(_m_path.data()) & FILE_ATTRIBUTE_DIRECTORY) {
        std::stringstream ss;
        ss << utils::utf8(_m_path);

        p = item_provider_factory::create(ss, this);
    }

    if (!p) {
        MessageBoxW(_m_hwnd, (L"Could not open " + _m_path).c_str(), L"Error", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    _m_providers.push(p);
}

void main_window::_fill_view() {
    item_provider* p = _m_providers.top();

    LVITEMW item { };
    item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

    static std::wstring empty;
    std::wstring tmp;
    
    for (size_t i = 0; i < p->count(); ++i) {
        list_item_data* data = new list_item_data;
        data->name = p->name(i);
        data->type = p->type(i);
        data->size = p->size(i);
        data->compression = p->compression(i);
        data->icon = p->icon(i);
        data->dir = p->dir(i);
        
        item.lParam = LPARAM(data);

        item.iItem = int(i);
        item.pszText = data->name.data();
        item.iImage = data->icon;

        int64_t size = data->size;
        tmp = utils::wbytes(size);
        
        //ListView_InsertItem(_m_left_list->handle(), &item);
        //ListView_SetItemText(_m_left_list->handle(), item.iItem, 1, data->type.data());
        //ListView_SetItemText(_m_left_list->handle(), item.iItem, 2, (size == 0) ? empty.data() : tmp.data());

        double compression = data->compression;
        tmp = std::to_wstring(int64_t(compression / 100.)) + L'%';

        //ListView_SetItemText(_m_left_list->handle(), item.iItem, 3, (compression == 0) ? empty.data() : tmp.data());
    }
}*/

void main_window::_set_left_sort_arrow(int col, sort_arrow type) const {
    /*HWND header = ListView_GetHeader(_m_left_list->handle());

    if (header) {
        HDITEMW hdr;
        hdr.mask = HDI_FORMAT;
        
        Header_GetItem(header, col, &hdr);

        switch (type) {
            case NO_ARROW:
                hdr.fmt = (hdr.fmt & ~(HDF_SORTDOWN | HDF_SORTUP));
                break;
            case UP_ARROW:
                hdr.fmt = (hdr.fmt & ~HDF_SORTDOWN) | HDF_SORTUP;
                break;
            case DOWN_ARROW:
                hdr.fmt = (hdr.fmt & ~HDF_SORTUP) | HDF_SORTDOWN;
                break;
        }

        Header_SetItem(header, col, &hdr);
    }*/
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

LRESULT main_window::_right_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);

            HBRUSH brush = CreateSolidBrush(RGB(0xff, 0, 0));

            FillRect(hdc, &rect, brush);

            DeleteObject(brush);

            EndPaint(hwnd, &ps);
            break;
        }

        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}
