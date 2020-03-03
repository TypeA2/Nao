#pragma once

namespace dims {
    static constexpr int64_t base_window_width = 1280;
    static constexpr int64_t base_window_height = 800;

    static constexpr int64_t gutter_size = 2;
    static constexpr int64_t control_height = 22;
    static constexpr int64_t control_button_width = 26;
    static constexpr int64_t browse_button_width = 73;

    static constexpr int64_t path_x_offset =
        control_button_width * 2 + gutter_size * 3;

    static constexpr int64_t play_button_size = 32;

    static constexpr int64_t volume_slider_width = 160;
    static constexpr int64_t volume_slider_height = 32;
}
