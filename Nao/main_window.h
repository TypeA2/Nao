#pragma once

#include "frameworks.h"

class main_window {
	public:
	main_window(HINSTANCE inst, int show_cmd);
	~main_window();

	// Program loop
	int run() const;

	private:
	// String size
	static constexpr size_t STRING_SIZE = 255;

	// Initialise stuff needed for the Windows API
	bool _init(int show_cmd);
	bool _register_class() const;
	bool _init_instance(int show_cmd);
	bool _create_subwindows();

	// Forwarded class WndProc function
	LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT _wnd_proc_left(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) const;

	// Message processing
	void _mouse_move(WPARAM wparam, LPARAM lparam);
	void _size(LPARAM lparam) const;
	void _left_size(LPARAM lparam) const;
	LRESULT _left_list_notify(NMHDR* nm) const;

	// Open a folder
	void _open_folder();

	// Update the left window's contents
	void _update_view();

	// Forwards the message processing to the member function
	static LRESULT CALLBACK _wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK _left_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK _right_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	
	// Static, doesn't require the member variables
	static INT_PTR CALLBACK _about(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	// Whether init succeeded
	bool _m_success;

	// Main instance
	HINSTANCE _m_inst;

	// Windows
	HWND _m_hwnd;
	HWND _m_left;
	HWND _m_right;
	HWND _m_left_path;
	HWND _m_left_list;

	// Strings
	WCHAR _m_title[STRING_SIZE];
	WCHAR _m_window_class[STRING_SIZE];
	WCHAR _m_left_window[STRING_SIZE];
	WCHAR _m_right_window[STRING_SIZE];

	// Accelerators
	HACCEL _m_accel;

	// Current path
	LPWSTR _m_path;

	// Constants
	static constexpr int gutter_size = 2;
	static constexpr int path_height = 22;
};
