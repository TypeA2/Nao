#pragma once

namespace dims {
    static constexpr int base_window_width = 1280;
    static constexpr int base_window_height = 800;

    static constexpr int gutter_size = 2;
    static constexpr int control_height = 22;
    static constexpr int control_button_width = 26;
    static constexpr int browse_button_width = 73;

    static constexpr int path_x_offset =
        control_button_width * 2 + gutter_size * 3;

    static constexpr int play_button_size = 32;

    static constexpr int volume_slider_width = 160;
    static constexpr int volume_slider_height = 32;
}
