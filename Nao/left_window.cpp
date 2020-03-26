#include "left_window.h"

#include "resource.h"

#include "utils.h"
#include "dimensions.h"

#include "list_view.h"
#include "line_edit.h"
#include "push_button.h"

#include "nao_view.h"

#include <string>

static rectangle start_rect(ui_element* parent) {
    auto [width, height] = parent->dims();
    return {
        .x = 0,
        .y = 0,
        .width = (width - dims::gutter_size) / 2,
        .height = height
    };
}

left_window::left_window(ui_element* parent, nao_view& view)
    : ui_element(parent, IDS_LEFT_WINDOW, start_rect(parent), win32::style | SS_SUNKEN)
    , _view { view }
    , _list { this, nao_view::list_view_header(), nao_view::shell_image_list() }
    , _up { this, win32::icon{} }
    , _refresh { this, win32::icon{} }
    , _browse { this, "Browse...", win32::icon{} }
    , _path { this } {

    _list.set_column_alignment(2, list_view::Right);

    _path.set_read_only(true);

    // Button icons
    win32::dynamic_library shell32("shell32.dll");

    _up.set_icon(shell32.load_icon_scaled(16817, { 16, 16 }));

    _refresh.set_icon(shell32.load_icon_scaled(16739, { 16, 16 }));
    _refresh.set_enabled(false);

    _browse.set_icon(shell32.load_icon_scaled(4, { 16, 16 }));

    defer_window_pos()
        .move(_up, { dims::gutter_size, dims::gutter_size,
            dims::control_button_width, dims::control_height + 2 })
        .move(_refresh, { dims::control_button_width + 2 * dims::gutter_size, dims::gutter_size,
            dims::control_button_width, dims::control_height + 2 });

    auto [x, y, w, h] = start_rect(parent);
    left_window::wm_size(0, { w, h });
}

line_edit& left_window::path() {
    return _path;
}

push_button& left_window::view_up() {
    return _up;
}

list_view& left_window::list() {
    return _list;
}

void left_window::wm_size(int, const dimensions& dims) {
    auto [width, height] = dims;

    defer_window_pos()
        .move(_path, { dims::path_x_offset, dims::gutter_size + 1,
            width - dims::path_x_offset - dims::browse_button_width - dims::gutter_size * 2, dims::control_height })
        .move(_list, { 0, dims::control_height + (dims::gutter_size * 2),
            width, height - (dims::gutter_size * 2) - dims::control_height })
        .move(_browse, { width - dims::browse_button_width - dims::gutter_size, dims::gutter_size,
            dims::browse_button_width, dims::control_height + 2 });
}

void left_window::wm_command(WORD, WORD, HWND target) {
    view_button_type btn = BUTTON_NONE;

    if (target == _up.handle()) {
        btn = BUTTON_UP;
    } else if (target == _browse.handle()) {
        btn = BUTTON_BROWSE;
    }

    if (btn != BUTTON_NONE) {
        _view.button_clicked(btn);
    }
}

void left_window::wm_notify(WPARAM, NMHDR* nm) {
    if (nm->hwndFrom == _list.handle()) {
        _view.list_clicked(nm);
    }
}
