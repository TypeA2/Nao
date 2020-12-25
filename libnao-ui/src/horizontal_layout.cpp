#include "horizontal_layout.h"

namespace nao {
    void horizontal_layout::add_element(window& element) {
        logger().debug("Adding child {}", fmt::ptr(&element));

        if (GetParent(element.handle()) != _handle) {
            SetParent(element.handle(), _handle);
        }

        SetWindowPos(element.handle(), nullptr, 50, 50, 50, 50, 0);

        _children.push_back(&element);
    }

}