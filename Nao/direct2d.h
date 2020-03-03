#pragma once

#include <d2d1.h>

#include "utils.h"

class ui_element;

namespace direct2d {
    class factory {
        com_ptr<ID2D1Factory> _factory;

        public:
        // Construct from a global instance
        explicit factory();

        // Construct from a specific instance
        explicit factory(const com_ptr<ID2D1Factory>& ptr);

        com_ptr<ID2D1HwndRenderTarget> create_hwnd_render_target(ui_element* element) const;
        com_ptr<ID2D1HwndRenderTarget> create_hwnd_render_target(HWND handle, dimensions dims) const;
    };
}
