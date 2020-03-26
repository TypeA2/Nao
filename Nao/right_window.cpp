#include "right_window.h"

#include "frameworks.h"
#include "resource.h"

#include "utils.h"
#include "dimensions.h"
#include "list_view.h"

static rectangle start_rect(ui_element* parent) {
    auto [width, height] = parent->dims();

    int64_t window_width = (width - dims::gutter_size) / 2;

    return {
        .x = window_width + dims::gutter_size,
        .y = 0,
        .width = window_width,
        .height = height
    };
}

right_window::right_window(ui_element* parent)
    : ui_element(parent, IDS_RIGHT_WINDOW, start_rect(parent), win32::style | SS_SUNKEN) {
   
}

void right_window::set_preview(preview_ptr instance) {
    _preview = std::move(instance);
}

void right_window::remove_preview() {
    _preview.reset();
}

preview* right_window::get_preview() const {
    return _preview.get();
}

void right_window::wm_size(int, const dimensions& dims) {
    if (_preview) {
        defer_window_pos()
            .move(_preview, { 0, 0, dims.width, dims.height });
    }
}
