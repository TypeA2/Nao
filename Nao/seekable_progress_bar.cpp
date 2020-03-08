#include "seekable_progress_bar.h"

#include "utils.h"

#include "resource.h"

#include <algorithm>

seekable_progress_bar::seekable_progress_bar(ui_element* parent, uintmax_t from, uintmax_t to)
    : ui_element(parent), _m_min { from }, _m_max { to }, _m_current { }
    , _m_old_cursor { LoadCursorW(nullptr, IDC_ARROW) } {
    ASSERT(from < to);

    std::wstring class_name = win32::load_wstring(IDS_PROGRESS_BAR);

    win32::register_once({
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = wnd_proc_fwd,
        .hInstance = win32::instance(),
        .hbrBackground = HBRUSH(COLOR_WINDOW + 1),
        .lpszClassName = class_name.c_str()
        });

    HWND handle = win32::create_window(class_name, L"", WS_CHILD | WS_VISIBLE, { },
        parent, new wnd_init(this, &seekable_progress_bar::_wnd_proc));

    ASSERT(handle);

    _m_hand_cursor = LoadCursorW(nullptr, IDC_HAND);

    _m_background_pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    _m_background_brush = CreateSolidBrush(RGB(0xff, 0xff, 0xff));

    _m_foreground_pen = CreatePen(PS_NULL, 0, 0);
    _m_foreground_brush = CreateSolidBrush(RGB(0x4d, 0xb2, 0xff));
}

seekable_progress_bar::~seekable_progress_bar() {
    DeleteObject(_m_background_pen);
    DeleteObject(_m_background_brush);

    DeleteObject(_m_foreground_pen);
    DeleteObject(_m_foreground_brush);
}

void seekable_progress_bar::set_progress(uintmax_t value) {
    ASSERT(value >= _m_min && value <= _m_max);

    _m_current = value;

    ASSERT(RedrawWindow(handle(), nullptr, nullptr, RDW_INVALIDATE));
}


void seekable_progress_bar::wm_paint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(handle(), &ps);

    auto [width, height] = dims();

    SelectObject(hdc, _m_background_pen);
    SelectObject(hdc, _m_background_brush);

    Rectangle(hdc, 0, 0, utils::narrow<int>(width), utils::narrow<int>(height));

    _draw_fill(hdc);

    EndPaint(handle(), &ps);
}


LRESULT seekable_progress_bar::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_MOUSEMOVE: {
            coordinates at {
                .x = GET_X_LPARAM(lparam),
                .y = GET_Y_LPARAM(lparam)
            };
            
            double frac = _m_current / static_cast<double>(_m_max);
            int64_t handle_pos = static_cast<int64_t>((width() - 1.) * frac);

            if (handle_pos >= (at.x - handle_size) && handle_pos <= (at.x + handle_size)) {
                SetCursor(_m_hand_cursor);
            } else {
                SetCursor(_m_old_cursor);
            }

            if (_m_is_dragging) {
                double pixel_frac = (at.x - 1.) / (width() - 1.);
                double target = round(_m_max * pixel_frac);
                if (target < _m_min) {
                    target = static_cast<double>(_m_min);
                } else if (target > _m_max) {
                    target = static_cast<double>(_m_max);
                }

                _m_current = static_cast<uintmax_t>(target);
                HDC hdc = device_context();
                _draw_fill(hdc, true);
                ReleaseDC(handle(), hdc);

                (void) parent()->send_message(PB_SEEK, _m_current);
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            coordinates at {
                .x = GET_X_LPARAM(lparam),
                .y = GET_Y_LPARAM(lparam)
            };

            double percent = _m_current / static_cast<double>(_m_max);
            int64_t handle_pos = static_cast<int64_t>((width() - 1i64) * percent);

            if (handle_pos >= (at.x - handle_size) && handle_pos <= (at.x + handle_size)) {
                _m_is_dragging = true;
                SetCapture(handle());

                (void) parent()->send_message(PB_CAPTURE, _m_current);
            }

            break;
        }

        case WM_LBUTTONUP: {
            if (_m_is_dragging) {
                _m_is_dragging = false;
                ReleaseCapture();

                (void) parent()->send_message(PB_RELEASE, _m_current);
            }
        }

        default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return 0;
}

void seekable_progress_bar::_draw_fill(HDC hdc, bool clear_nondrawn) const {
    HGDIOBJ old_pen = SelectObject(hdc, _m_foreground_pen);
    HGDIOBJ old_brush = SelectObject(hdc, _m_foreground_brush);

    auto [width, height] = dims();

    double percent = _m_current / static_cast<double>(_m_max);
    int drawn_width = utils::narrow<int>(round(width * percent));

    Rectangle(hdc, 1, 1, drawn_width, utils::narrow<int>(height));

    if (clear_nondrawn) {
        SelectObject(hdc, win32::stock_object(NULL_PEN));
        SelectObject(hdc, _m_background_brush);
        Rectangle(hdc, std::max(drawn_width, 1), 1, utils::narrow<int>(width), utils::narrow<int>(height));
    }

    SelectObject(hdc, old_pen);
    SelectObject(hdc, old_brush);
}

