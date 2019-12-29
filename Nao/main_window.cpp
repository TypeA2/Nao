#include "main_window.h"

#include "Resource.h"

#include <cstdlib>

main_window::main_window(HINSTANCE inst, int show_cmd)
	: _m_success(true)
	, _m_inst(inst) {
	// Whether the setup was a success
	_m_success = _init(show_cmd);
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

	return RegisterClassExW(&wcex) != 0;
}

bool main_window::_init_instance(int show_cmd) {
	// Create the main window
	HWND hwnd = CreateWindowW(_m_window_class, _m_title, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, _m_inst, nullptr);

	if (!hwnd) {
		return false;
	}

	// Set user data to a pointer to this instance
	SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// Continue
	ShowWindow(hwnd, show_cmd);
	UpdateWindow(hwnd);
	return true;
}

LRESULT main_window::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) const {
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
				default:
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

		default:
			return DefWindowProcW(hwnd, msg, wparam, lparam);
	}

	return EXIT_SUCCESS;
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

