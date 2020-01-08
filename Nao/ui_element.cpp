#include "ui_element.h"

#include "utils.h"

#include <cassert>

ui_element::ui_element(ui_element* parent)
	: _m_parent { parent }
	, _m_handle { }
	, _m_mem_wnd_proc { }
	, _m_default_wnd_proc { }
	, _m_wnd_proc_ctx { } {
	
}

ui_element::~ui_element() {
	destroy();
}

bool ui_element::destroy() {
	HWND handle = _m_handle;
	_m_handle = nullptr;
	return DestroyWindow(handle);
}

ui_element* ui_element::parent() const {
	return _m_parent;
}

HWND ui_element::handle() const {
	return _m_handle;
}

long ui_element::width() const {
	RECT rect;
	assert(GetWindowRect(handle(), &rect));

	return rect.right - rect.left;
}

long ui_element::height() const {
	RECT rect;
	assert(GetWindowRect(handle(), &rect));

	return rect.bottom - rect.top;
}

bool ui_element::move(int x, int y, int width, int height) const {
	return SetWindowPos(handle(), nullptr, x, y, width, height, 0);
}


HDWP& ui_element::move_dwp(HDWP& dwp, int x, int y, int width, int height) const {
	dwp = DeferWindowPos(dwp, handle(), nullptr, x, y, width, height, 0);

	return dwp;
}

void ui_element::set_handle(HWND handle) {
	_m_handle = handle;
}

void ui_element::use_wnd_proc(member_wnd_proc<> new_proc) {
	_m_mem_wnd_proc = new_proc;

	SetWindowLongPtrW(handle(), GWLP_USERDATA, LONG_PTR(this));

	/*
	 * Set the new WndProc to our forwarding function, which calls the member
	 * WndProc if it is set.
	 */
	_m_default_wnd_proc = WNDPROC(
		SetWindowLongPtrW(handle(), GWLP_WNDPROC, LONG_PTR(wnd_proc_fwd)));
}

WNDPROC ui_element::default_wnd_proc() const {
	return _m_default_wnd_proc;
}

LRESULT ui_element::wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// Retrieve instance
	ui_element* _this = reinterpret_cast<ui_element*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

	if (_this && _this->_m_mem_wnd_proc) {
		return ((*_this).*(_this->_m_mem_wnd_proc))(hwnd, msg, wparam, lparam);
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}
