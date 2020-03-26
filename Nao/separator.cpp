#include "separator.h"
#include "utils.h"

separator::separator(ui_element* parent, separator_type type) : ui_element(parent, WC_STATICW, win32::style | type) {

}
