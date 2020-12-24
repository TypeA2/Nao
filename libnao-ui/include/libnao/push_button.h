#pragma once

#include "window.h"

namespace nao {
    class push_button : public window {
        public:
        push_button(window* parent, std::string_view text);
    };
}