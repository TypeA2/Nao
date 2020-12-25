#pragma once

#include "window.h"

namespace nao {
    class layout;

    class push_button : public window {
        public:
        push_button(std::string_view text, layout& parent);
    };
}