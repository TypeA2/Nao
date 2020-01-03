#include "main_window.h"

#include "resource.h"

#include "utils.h"

#include <cstdlib>
#include <ShObjIdl.h>
#include <Uxtheme.h>

main_window::main_window(HINSTANCE inst, int show_cmd)
	: _m_success(true)
	, _m_inst(inst)
	, _m_path(nullptr) {
	// Whether the setup was a success
	_m_success = _init(show_cmd);

#ifdef _DEBUG
	WCHAR path[] = L"D:\\Steam\\steamapps\\common\\NieRAutomata";
	_m_path = LPWSTR(CoTaskMemAlloc(sizeof(path)));
	memcpy(_m_path, path, sizeof(path));
	_update_view();
#endif
}

main_window::~main_window() {
	if (_m_path) {
		CoTaskMemFree(_m_path);
	}
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

bool main_window::_init(int show_cmd) {
	// Load string resources
	LoadStringW(_m_inst, IDS_APP_TITLE, _m_title, STRING_SIZE);
	LoadStringW(_m_inst, IDC_NAO, _m_window_class, STRING_SIZE);
	LoadStringW(_m_inst, IDS_LEFT_WINDOW, _m_left_window, STRING_SIZE);
	LoadStringW(_m_inst, IDS_RIGHT_WINDOW, _m_right_window, STRING_SIZE);

	// Additional setup
	if (!_register_class() || !_init_instance(show_cmd)) {
		return false;
	}

	// Load accelerators
	_m_accel = LoadAcceleratorsW(_m_inst, MAKEINTRESOURCEW(IDC_NAO));
	
	return _m_accel != nullptr;
}

bool main_window::_register_class() const {
	// Our window class instance
	WNDCLASSEXW wcex {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		_wnd_proc_fwd,
		0,
		0,
		_m_inst,
		LoadIconW(_m_inst, MAKEINTRESOURCEW(IDI_NAO)),
		LoadCursorW(nullptr, IDC_ARROW),
		HBRUSH(COLOR_WINDOW + 1),
		MAKEINTRESOURCEW(IDC_NAO),
		_m_window_class,
		LoadIconW(_m_inst, MAKEINTRESOURCEW(IDI_NAO))
	};

	if (RegisterClassExW(&wcex) == 0) {
		return false;
	}

	// Left window class

	wcex.lpfnWndProc = _left_proc_fwd;
	wcex.lpszClassName = _m_left_window;

	if (RegisterClassExW(&wcex) == 0) {
		return false;
	}

	// Right window
	wcex.lpfnWndProc = _right_proc_fwd;
	wcex.lpszClassName = _m_right_window;

	return RegisterClassExW(&wcex) != 0;
}

bool main_window::_init_instance(int show_cmd) {
	int nx_size = GetSystemMetrics(SM_CXSCREEN);
	int ny_size = GetSystemMetrics(SM_CYSCREEN);

	int nx_pos = (nx_size - 1280) / 2;
	int ny_pos = (ny_size - 960) / 2;
	
	// Create the main window
	_m_hwnd = CreateWindowExW(0, _m_window_class, _m_title,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		nx_pos, ny_pos, 1280, 800, nullptr, nullptr, _m_inst, nullptr);

	if (!_m_hwnd) {
		return false;
	}

	// Set user data to a pointer to this instance
	SetWindowLongPtrW(_m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// Continue
	UpdateWindow(_m_hwnd);

	// WM_CREATE is ignored because of the forwarding mechanism
	SendMessageW(_m_hwnd, WM_CREATE, 0, 0);
	return true;
}

bool main_window::_create_subwindows() {
	RECT rect;
	GetClientRect(_m_hwnd, &rect);
	
	HFONT font = HFONT(GetStockObject(DEFAULT_GUI_FONT));

	HMODULE shell32 = LoadLibraryW(L"shell32.dll");
	if (!shell32) {
		return false;
	}

	HICON up_icon;
	LoadIconWithScaleDown(shell32, MAKEINTRESOURCEW(16817), 16, 16, &up_icon);

	HICON refresh_icon;
	LoadIconWithScaleDown(shell32, MAKEINTRESOURCEW(16739), 16, 16, &refresh_icon);

	HICON folder_icon;
	LoadIconWithScaleDown(shell32, MAKEINTRESOURCEW(4), 16, 16, &folder_icon);


	int window_width = (rect.right - gutter_size) / 2;

	_m_left = CreateWindowExW(0, _m_left_window, L"",
		WS_CHILD | WS_VISIBLE | SS_SUNKEN,
		0, 0,
		window_width, rect.bottom,
		_m_hwnd, nullptr, _m_inst, nullptr);

	if (!_m_left) {
		return false;
	}

	SetWindowLongPtrW(_m_left, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	_m_right = CreateWindowExW(0, _m_right_window, L"",
		WS_CHILD | WS_VISIBLE | SS_SUNKEN,
		window_width + gutter_size, 0,
		window_width, rect.bottom,
		_m_hwnd, nullptr, _m_inst, nullptr);

	if (!_m_right) {
		return false;
	}



	// Folder up
	_m_left_up = CreateWindowExW(0, WC_BUTTONW, L"",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_ICON,
		gutter_size, gutter_size, control_button_width, control_height + 2,
		_m_left, nullptr, _m_inst, nullptr);

	if (!_m_left_up) {
		return false;
	}

	// Reload view
	_m_left_refresh = CreateWindowExW(0, WC_BUTTONW, L"",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_ICON,
		// control_height + 2 because the border is within the window
		control_button_width + 2 * gutter_size, gutter_size, control_button_width, control_height + 2,
		_m_left, nullptr, _m_inst, nullptr);

	if (!_m_left_refresh) {
		return false;
	}

	// Browse button
	_m_left_browse = CreateWindowExW(0, WC_BUTTONW, L"Browse...",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		window_width - browse_button_width - gutter_size, gutter_size, browse_button_width, control_height + 2,
		_m_left, nullptr, _m_inst, nullptr);

	if (!_m_left_browse) {
		return false;
	}

	SendMessageW(_m_left_browse, WM_SETFONT, WPARAM(font), true);
	
	// Button icons
	SendMessageW(_m_left_up, BM_SETIMAGE, IMAGE_ICON, LPARAM(up_icon));
	SendMessageW(_m_left_refresh, BM_SETIMAGE, IMAGE_ICON, LPARAM(refresh_icon));
	SendMessageW(_m_left_browse, BM_SETIMAGE, IMAGE_ICON, LPARAM(folder_icon));

	// Current path edit control
	_m_left_path = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, L"",
		WS_CHILD | WS_VISIBLE |
		ES_LEFT,
		// gutter_size + 1 to accomodate the border
		path_x_offset, gutter_size + 1, window_width - path_x_offset - browse_button_width - gutter_size * 2, control_height,
		_m_left, nullptr, _m_inst, nullptr);

	if (!_m_left_path) {
		return false;
	}

	SendMessageW(_m_left_path, WM_SETFONT, WPARAM(font), true);

	// Filesystem list view
	_m_left_list = CreateWindowExW(0,
		WC_LISTVIEWW, L"",
		WS_CHILD | WS_VISIBLE |
		LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
		0, control_height + 2 * gutter_size, window_width, rect.bottom,
		_m_left, nullptr, _m_inst, nullptr);

	if (!_m_left_list) {
		return false;
	}

	SetWindowTheme(_m_left_list, L"Explorer", nullptr);
	ListView_SetExtendedListViewStyle(_m_left_list, LVS_EX_FULLROWSELECT);

	// And it's columns
	{
		LPCWSTR titles[] = { L"Name", L"Size", L"Type", L"Compressed" };
		LVCOLUMNW col;
		col.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
		col.cx = window_width / 4;
		col.fmt = LVCFMT_LEFT;

		for (size_t i = 0; i < std::size(titles); ++i) {
			col.iOrder = int(i);

			size_t header_length = wcslen(titles[i]) + 1;
			col.pszText = new WCHAR[header_length]();
			wcscpy_s(col.pszText, header_length, titles[i]);

			if (i == std::size(titles) - 1) {
				// Pad with last item
				col.cx = (window_width + 3) / 4;
			}

			ListView_InsertColumn(_m_left_list, i, &col);
		}
	}

	DeleteObject(up_icon);
	DeleteObject(refresh_icon);
	DeleteObject(folder_icon);

	FreeLibrary(shell32);
	
	return true;
}

LRESULT main_window::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// Message processing
	switch (msg) {
		case WM_CREATE: {
			if (!_create_subwindows()) {
				PostQuitMessage(EXIT_FAILURE);
				return false;
			}
			break;
		}
		case WM_COMMAND: {
			switch (LOWORD(wparam)) {
				case IDM_ABOUT:
					DialogBoxW(_m_inst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hwnd, _about);
					break;
				case IDM_EXIT:
					DestroyWindow(hwnd);
					break;
				case ID_FILE_OPEN:
					_open_folder();
					break;
				default:
					utils::coutln(LOWORD(wparam));
					return DefWindowProcW(hwnd, msg, wparam, lparam);
			}
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			break;
		}

		case WM_DESTROY:
			PostQuitMessage(EXIT_SUCCESS);
			break;

		case WM_MOUSEMOVE:
			_mouse_move(wparam, lparam);
			break;

		case WM_SIZE:
			_size(lparam);
			break;

		default:
			return DefWindowProcW(hwnd, msg, wparam, lparam);
	}

	return EXIT_SUCCESS;
}

LRESULT main_window::_wnd_proc_left(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
		case WM_SIZE:
			_left_size(lparam);
			break;

		case WM_NOTIFY: {
			NMHDR* nm = reinterpret_cast<NMHDR*>(lparam);

			if (nm->hwndFrom == _m_left_list) {
				return _left_list_notify(nm);
			}
			break;
		}

		case WM_COMMAND:
			if (HWND(lparam) == _m_left_browse) {
				_open_folder();
			}
			break;
			
		default:
			return DefWindowProcW(hwnd, msg, wparam, lparam);
	}
	
	return EXIT_SUCCESS;
}

void main_window::_mouse_move(WPARAM wparam, LPARAM lparam) {
	(void) wparam;
	(void) lparam;
	(void) this;
}

void main_window::_size(LPARAM lparam) const {
	WORD new_width = LOWORD(lparam);
	WORD new_height = HIWORD(lparam);

	int window_width = (new_width - gutter_size) / 2;
	
	HDWP dwp = BeginDeferWindowPos(2);
	dwp = DeferWindowPos(dwp, _m_left, nullptr, 
		0, 0, window_width, new_height, 0);
	dwp = DeferWindowPos(dwp, _m_right, nullptr,
		window_width + gutter_size, 0, window_width, new_height, 0);
		
	EndDeferWindowPos(dwp);
}

void main_window::_left_size(LPARAM lparam) const {
	WORD new_width = LOWORD(lparam);
	WORD new_height = HIWORD(lparam);

	HDWP dwp = BeginDeferWindowPos(2);
	dwp = DeferWindowPos(dwp, _m_left_path, nullptr,
		path_x_offset, gutter_size + 1, new_width - path_x_offset - gutter_size, control_height, 0);
	dwp = DeferWindowPos(dwp, _m_left_list, nullptr,
		0, control_height + (gutter_size * 2),
		new_width, new_height - (gutter_size * 2) - control_height, 0);

	EndDeferWindowPos(dwp);
}

LRESULT main_window::_left_list_notify(NMHDR* nm) const {
	const wchar_t* data[3][4] = {
		{ L"Foo", L"28 KiB", L"Dunno", L"No" },
		{ L"Bar", L"18 MiB", L"Yes", L"95%" },
		{ L"Baz", L"3.53 GiB", L"Maybe", L"32%" }
	};
	switch (nm->code) {
		case LVN_GETDISPINFOW: {
			NMLVDISPINFOW* pdi = reinterpret_cast<NMLVDISPINFOW*>(nm);
			LVITEMW& item = pdi->item;

			if (item.mask & LVIF_TEXT) {
				size_t len = wcslen(data[item.iItem][item.iSubItem]) + 1;
				item.pszText = new WCHAR[len + 1];
				wcscpy_s(item.pszText, len, data[item.iItem][item.iSubItem]);
			} else {
				utils::coutln("getdispinfo");
			}

			return true;
		}
	}

	return false;
}

void main_window::_open_folder() {
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

		hr = dialog->Show(_m_hwnd);
		if (SUCCEEDED(hr)) {
			IShellItem* item;
			hr = dialog->GetResult(&item);
			if (SUCCEEDED(hr)) {
				hr = item->GetDisplayName(SIGDN_FILESYSPATH, &_m_path);
				if (FAILED(hr)) {
					utils::coutln("Failed to get path");
				} else {
					_update_view();
				}

				item->Release();
			}
		}
	}

	dialog->Release();
}

void main_window::_update_view() {
	SendMessageW(_m_left_path, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(_m_path));

	//SendMessageW(_m_left_list, LVM_SETCALLBACKMASK)
	static bool what = false;

	if (!what) {
		LVITEMW item { };
		item.pszText = LPSTR_TEXTCALLBACKW;
		item.mask = LVIF_TEXT;

		if (FAILED(ListView_InsertItem(_m_left_list, &item))) {
			utils::coutln("failed insert");
		}

		item.iItem = 1;
		if (FAILED(ListView_InsertItem(_m_left_list, &item))) {
			utils::coutln("failed insert");
		}

		item.iItem = 2;
		if (FAILED(ListView_InsertItem(_m_left_list, &item))) {
			utils::coutln("failed insert");
		}
		what = true;
	}

	
	UpdateWindow(_m_left_list);
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

LRESULT main_window::_wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// Retrieve the instance pointer
	main_window* _this = reinterpret_cast<main_window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

	if (_this) {
		// Forward processing to the instance
		return _this->_wnd_proc(hwnd, msg, wparam, lparam);
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}

LRESULT main_window::_left_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// Retrieve the instance pointer
	main_window* _this = reinterpret_cast<main_window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

	if (_this) {
		// Forward processing to the instance
		return _this->_wnd_proc_left(hwnd, msg, wparam, lparam);
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
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
