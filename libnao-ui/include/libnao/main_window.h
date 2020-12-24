#pragma once

#include "window.h"

namespace nao {
    class main_window : public window {
        public:
        main_window(std::string_view title = "");

        void set_title(std::string_view title) const;
    };
}