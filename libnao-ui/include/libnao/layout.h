#pragma once

#include "window.h"

#include <libnao/logging.h>

namespace nao {
    class main_window;

    class layout : public window {
        NAO_LOGGER(layout)

        public:
        layout(main_window& w);

        virtual void add_element(window& element) = 0;

        private:
        friend class main_window;
        void _set_parent(main_window& w) const;
    };
}
