#include "slider.h"

#include "utils.h"

slider::slider(ui_element* parent, int64_t min, int64_t max, bool vertical)
    : ui_element(parent, TRACKBAR_CLASSW, win32::style | (vertical ? TBS_VERT : 0)) {

    static bool has_edited_class = false;
    if (!has_edited_class) {
        SetClassLongPtrW(handle(), GCLP_HBRBACKGROUND, COLOR_WINDOW + 1);

        has_edited_class = true;
    }

    (void) send_message(TBM_SETRANGEMIN, false, min);
    (void) send_message(TBM_SETRANGEMAX, true, max);
}

void slider::set_position(int64_t val) const {
    (void) send_message(TBM_SETPOS, true, val);
}

int64_t slider::get_position() const {
    return send_message(TBM_GETPOS);
}
