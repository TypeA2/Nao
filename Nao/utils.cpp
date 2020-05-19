#include "utils.h"

#include "frameworks.h"

#include <iomanip>

coordinates coordinates::from_lparam(LPARAM lparam) {
    return {
        .x = GET_X_LPARAM(lparam),
        .y = GET_Y_LPARAM(lparam)
    };
}


rectangle dimensions::rect() const {
    return {
        .x = 0,
        .y = 0,
        .width = width,
        .height = height
    };
}

dimensions dimensions::from_lparam(LPARAM lparam) {
    return {
        .width = LOWORD(lparam),
        .height = HIWORD(lparam)
    };
}


