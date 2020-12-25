#pragma once

#include "layout.h"

#include <vector>

namespace nao {
    class horizontal_layout : public layout {
        NAO_LOGGER(horizontal_layout)

        std::vector<window*> _children;
        public:
        using layout::layout;

        void add_element(window& element) override;
    };
}