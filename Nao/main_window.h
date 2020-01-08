#pragma once

#include "frameworks.h"

#include "list_view.h"

#include <string>
#include <map>
#include <stack>

class item_provider;

class main_window : public ui_element {
	public:
	main_window(HINSTANCE inst, int show_cmd);
	~main_window();

	// Program loop
	int run() const;

	// Getters
	HWND hwnd() const;

	// Some public types
	using icon_index = std::pair<std::wstring, int>;

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
	void _list_sort(int col);

	// Open a folder
	void _open_folder();

	// Move to a specific (non-relative) path
	void _move_to(std::wstring path);

	// Update the left window's contents
	void _update_view();
	void _get_provider();
	void _fill_view();

	// Set left list sort arrow
	enum sort_arrow {
		NO_ARROW,
		UP_ARROW,
		DOWN_ARROW
	};
	void _set_left_sort_arrow(int col, sort_arrow type) const;

	// Forwards the message processing to the member function
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
	//HWND _m_left_list;
	//IImageList* _m_left_image_list;
	//
	list_view* _m_left_list;

	// Strings
	WCHAR _m_title[STRING_SIZE];
	WCHAR _m_window_class[STRING_SIZE];
	WCHAR _m_left_window[STRING_SIZE];
	WCHAR _m_right_window[STRING_SIZE];

	// Accelerators
	HACCEL _m_accel;

	// Current path
	std::wstring _m_path;
	
	std::stack<item_provider*> _m_providers;

	// Column sort
	enum sort_order : int8_t {
		NONE,
		NORMAL,
		REVERSE
	};
	sort_order _m_sort_order[4];
	int _m_selected_col;

	// LPARAM for list items
	struct list_item_data {
		std::wstring name;
		std::wstring type;
		int64_t size {};
		double compression {};
		int icon {};
		bool dir {};
	};

	// Constants
	static constexpr int gutter_size = 2;
	static constexpr int control_height = 22;
	static constexpr int control_button_width = 26;
	static constexpr int browse_button_width = 73;
	static constexpr int path_x_offset = control_button_width * 2 + gutter_size * 3;

	// Default order when unselected columns get selected
	static constexpr sort_order default_order[4]
		= { NORMAL, NORMAL, REVERSE, REVERSE };
};
