#pragma once

#include "frameworks.h"

#include <vector>
#include <string>
#include <map>

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
	LRESULT _wnd_proc_left(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	// Message processing
	void _mouse_move(WPARAM wparam, LPARAM lparam);
	void _size(LPARAM lparam) const;
	void _left_size(LPARAM lparam) const;
	LRESULT _left_list_notify(NMHDR* nm) const;

	// Open a folder
	void _open_folder();

	// Update the left window's contents
	void _update_view();

	// Fill view from the filesystem
	void _fill_from_fs(const std::wstring& dir);

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
	HWND _m_left_up;
	HWND _m_left_refresh;
	HWND _m_left_browse;
	HWND _m_left_path;
	HWND _m_left_list;
	HIMAGELIST _m_left_image_list;

	// Strings
	WCHAR _m_title[STRING_SIZE];
	WCHAR _m_window_class[STRING_SIZE];
	WCHAR _m_left_window[STRING_SIZE];
	WCHAR _m_right_window[STRING_SIZE];

	// Accelerators
	HACCEL _m_accel;

	// Current path
	std::wstring _m_path;

	// Current path exists on the filesystem
	bool _m_on_filesystem;

	// Current filesystem files and folders
	struct fs_entry {
		bool is_dir;
		std::wstring name;
		std::wstring type;
		int64_t size;
		std::wstring size_str;
		int icon_index;
	};
	std::vector<fs_entry> _m_dirs;
	std::vector<fs_entry> _m_files;

	// Per-type icons, source + index
	using icon_index = std::pair<std::wstring, int>;
	std::map<icon_index, int> _m_icons;

	// Constants
	static constexpr int gutter_size = 2;
	static constexpr int control_height = 22;
	static constexpr int control_button_width = 26;
	static constexpr int browse_button_width = 73;
	static constexpr int path_x_offset = control_button_width * 2 + gutter_size * 3;

	static constexpr int max_list_elements = 1024;
};
