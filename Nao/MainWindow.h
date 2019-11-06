#pragma once

#include "Frameworks.h"

class MainWindow {
	public:
	MainWindow(HINSTANCE inst, int show_cmd);

	// Program loop
	int run() const;

	private:
	// String size
	static constexpr size_t STRING_SIZE = 255;

	// Initialise stuff needed for the Windows API
	bool _init(int show_cmd);
	bool _register_class() const;
	bool _init_instance(int show_cmd);

	// Forwarded class WndProc function
	LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) const;

	// Forwards the message processing to the member function
	static LRESULT CALLBACK _wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	// Static, doesn't require the member variables
	static INT_PTR CALLBACK _about(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	// Whether init succeeded
	bool _m_success;

	// Main instance
	HINSTANCE _m_inst;

	// Strings
	WCHAR _m_title[STRING_SIZE];
	WCHAR _m_window_class[STRING_SIZE];

	// Accelerators
	HACCEL _m_accel;
};
