#include "main_window.h"

#include "resource.h"

#include "utils.h"
#include "item_provider.h"
#include "item_provider_factory.h"

#include <process.h>

#include <clocale>
#include <algorithm>

main_window::main_window(HINSTANCE inst, int show_cmd)
	: _m_success(false)
	, _m_inst(inst)

	, _m_hwnd {}	  , _m_left {}		  , _m_right {}
	, _m_left_up {}	  , _m_left_refresh {}, _m_left_browse {}
	, _m_left_path {} , _m_left_list {}	  , _m_left_image_list {}

	, _m_title { 0 }	  , _m_window_class { 0 }
	, _m_left_window { 0 }, _m_right_window { 0 }
	
	, _m_accel {}
	, _m_sort_order {}, _m_selected_col {} {
	_m_inst = inst;
	if (!_m_inst) {
		return;
	}
	
	// Whether the setup was a success
	_m_success = _init(show_cmd);

#ifdef _DEBUG
	_move_to(L"D:\\Steam\\steamapps\\common\\NieRAutomata");
#endif
}

main_window::~main_window() {
	if (_m_left_image_list) {
		_m_left_image_list->Release();
	}

	while (!_m_providers.empty()) {
		item_provider* p = _m_providers.top();
		_m_providers.pop();

		delete p;
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

HWND main_window::hwnd() const {
	return _m_hwnd;
}



bool main_window::_init(int show_cmd) {
	// CRT locale
	std::setlocale(LC_ALL, "en_US.utf8");
	
	// Load string resources
	LoadStringW(_m_inst, IDS_APP_TITLE, _m_title, STRING_SIZE);
	LoadStringW(_m_inst, IDC_NAO, _m_window_class, STRING_SIZE);
	LoadStringW(_m_inst, IDS_LEFT_WINDOW, _m_left_window, STRING_SIZE);
	LoadStringW(_m_inst, IDS_RIGHT_WINDOW, _m_right_window, STRING_SIZE);

	// Additional setup
	if (!_register_class() || !_init_instance(show_cmd)) {
		utils::coutln("_register_class or _init_instance failed");
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
	(void) show_cmd;
	int nx_size = GetSystemMetrics(SM_CXSCREEN);
	int ny_size = GetSystemMetrics(SM_CYSCREEN);

	int nx_pos = (nx_size - 1280) / 2;
	int ny_pos = (ny_size - 960) / 2;
	
	// Create the main window
	_m_hwnd = CreateWindowExW(0, _m_window_class, _m_title,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		nx_pos, ny_pos, 1280, 800, nullptr, nullptr, _m_inst, nullptr);

	if (!_m_hwnd) {
		utils::coutln("main window creation failed");
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
		utils::coutln("loading shell32.dll failed");
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
		utils::coutln("creating left window failed");
		return false;
	}

	SetWindowLongPtrW(_m_left, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	_m_right = CreateWindowExW(0, _m_right_window, L"",
		WS_CHILD | WS_VISIBLE | SS_SUNKEN,
		window_width + gutter_size, 0,
		window_width, rect.bottom,
		_m_hwnd, nullptr, _m_inst, nullptr);

	if (!_m_right) {
		utils::coutln("creating right window failed");
		return false;
	}



	// Folder up
	_m_left_up = CreateWindowExW(0, WC_BUTTONW, L"",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_ICON | WS_DISABLED,
		gutter_size, gutter_size, control_button_width, control_height + 2,
		_m_left, nullptr, _m_inst, nullptr);

	if (!_m_left_up) {
		utils::coutln("creating left up button failed");
		return false;
	}

	// Reload view
	_m_left_refresh = CreateWindowExW(0, WC_BUTTONW, L"",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_ICON | WS_DISABLED,
		// control_height + 2 because the border is within the window
		control_button_width + 2 * gutter_size, gutter_size, control_button_width, control_height + 2,
		_m_left, nullptr, _m_inst, nullptr);

	if (!_m_left_refresh) {
		utils::coutln("creating left refresh button failed");
		return false;
	}

	// Browse button
	_m_left_browse = CreateWindowExW(0, WC_BUTTONW, L"Browse...",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		window_width - browse_button_width - gutter_size, gutter_size, browse_button_width, control_height + 2,
		_m_left, nullptr, _m_inst, nullptr);

	if (!_m_left_browse) {
		utils::coutln("creating left browse button failed");
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
		utils::coutln("creating left path edit failed");
		return false;
	}

	SendMessageW(_m_left_path, WM_SETFONT, WPARAM(font), true);

	if (!_left_list_setup(window_width, rect.bottom)) {
		return false;
	}
	
	DeleteObject(up_icon);
	DeleteObject(refresh_icon);
	DeleteObject(folder_icon);

	FreeLibrary(shell32);

	return true;
}

bool main_window::_left_list_setup(int window_width,int window_height) {
	// Filesystem list view
	_m_left_list = CreateWindowExW(0,
		WC_LISTVIEWW, L"",
		WS_CHILD | WS_VISIBLE |
		LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
		0, control_height + 2 * gutter_size, window_width, window_height,
		_m_left, nullptr, _m_inst, nullptr);

	if (!_m_left_list) {
		utils::coutln("creating left list failed");
		return false;
	}
	
	SetWindowTheme(_m_left_list, L"Explorer", nullptr);
	ListView_SetExtendedListViewStyle(_m_left_list, LVS_EX_FULLROWSELECT);

	// And it's columns
	{
		LPCWSTR titles[] = { L"Name", L"Type", L"Size", L"Compressed" };
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

	// And the image list
	if (FAILED(SHGetImageList(SHIL_SMALL, IID_PPV_ARGS(&_m_left_image_list)))) {
		utils::coutln("failed to retrieve shell image list");
		return false;
	}

	ListView_SetImageList(_m_left_list, _m_left_image_list, LVSIL_SMALL);

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
					utils::coutln("WM_COMMAND", LOWORD(wparam));
					return DefWindowProcW(hwnd, msg, wparam, lparam);
			}
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			(void) hdc;
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
				switch (nm->code) {
					case LVN_COLUMNCLICK: {
						NMLISTVIEW* view = reinterpret_cast<NMLISTVIEW*>(nm);
						if (_m_selected_col != view->iSubItem) {
							// Set default order
							_m_sort_order[view->iSubItem] = default_order[view->iSubItem];
						} else {
							_m_sort_order[_m_selected_col]
								= (_m_sort_order[_m_selected_col] == REVERSE) ? NORMAL : REVERSE;
						}
						
						_m_selected_col = view->iSubItem;
						_list_sort(_m_selected_col);
						break;
					}
					
					default:
						break;
				}
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

	HDWP dwp = BeginDeferWindowPos(3);
	dwp = DeferWindowPos(dwp, _m_left_path, nullptr,
		path_x_offset, gutter_size + 1, new_width - path_x_offset - browse_button_width - gutter_size * 2, control_height, 0);
	dwp = DeferWindowPos(dwp, _m_left_list, nullptr,
		0, control_height + (gutter_size * 2),
		new_width, new_height - (gutter_size * 2) - control_height, 0);
	dwp = DeferWindowPos(dwp, _m_left_browse, nullptr,
		new_width - browse_button_width - gutter_size, gutter_size, browse_button_width, control_height + 2, 0);

	EndDeferWindowPos(dwp);
}

void main_window::_list_sort(int col) {
	for (int i = 0; i < int(std::size(_m_sort_order)); ++i) {
		if (i == col) {
			_set_left_sort_arrow(i,
				(_m_sort_order[i] == REVERSE) ? DOWN_ARROW : UP_ARROW);
		} else {
			_set_left_sort_arrow(i, NO_ARROW);
		}
	}

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
	
	ListView_SortItems(_m_left_list, comp, MAKELPARAM(col, _m_sort_order[col]));
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

void main_window::_move_to(std::wstring path) {
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
			_m_path = path;
			_update_view();
		} else {
			MessageBoxW(_m_hwnd, L"Failed to find existing folder in path", L"Error",
				MB_OK | MB_ICONEXCLAMATION);
		}
	}
}

void main_window::_update_view() {
	SendMessageW(_m_left_path, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(_m_path.data()));

	void(__cdecl * fwd)(void*) = [](void* args) {
		(void) CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		
		main_window* _this = reinterpret_cast<main_window*>(args);
		_this->_get_provider();
		_this->_fill_view();
		_this->_list_sort(_this->_m_selected_col);

		CoUninitialize();
	};

	size_t low;
	size_t high;
	GetCurrentThreadStackLimits(&low, &high);

	_beginthread(fwd, unsigned(high - low), this);

}

void main_window::_get_provider() {
	item_provider* p = nullptr;
	if (GetFileAttributesW(_m_path.data()) & FILE_ATTRIBUTE_DIRECTORY) {
		size_t required;
		wcstombs_s(&required, nullptr, 0, _m_path.data(), 0);

		std::string path(required - 1, '\0');
		wcstombs_s(&required, path.data(), required, _m_path.data(), _TRUNCATE);
		std::stringstream ss;
		ss << path;

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
		
		ListView_InsertItem(_m_left_list, &item);
		ListView_SetItemText(_m_left_list, item.iItem, 1, data->type.data());
		ListView_SetItemText(_m_left_list, item.iItem, 2, (size == 0) ? empty.data() : tmp.data());

		double compression = data->compression;
		tmp = std::to_wstring(int64_t(compression / 100.)) + L'%';

		ListView_SetItemText(_m_left_list, item.iItem, 3, (compression == 0) ? empty.data() : tmp.data());
	}
}

void main_window::_set_left_sort_arrow(int col, sort_arrow type) const {
	HWND header = ListView_GetHeader(_m_left_list);

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
	}
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
