#pragma once

#include "window.h"

namespace nao {
    class layout;

    class main_window : public window {
        NAO_LOGGER(main_window)

        std::vector<layout*> _layouts;

        public:
        main_window(std::string_view title = "");

        void set_title(std::string_view title) const;

        void add_layout(layout& l);
    };
}