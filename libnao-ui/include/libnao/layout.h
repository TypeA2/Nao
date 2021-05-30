/**
 *  This file is part of libnao-ui.
 *
 *  libnao-ui is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-ui is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-ui.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "window.h"

#include <libnao/logging.h>

namespace nao {
    class main_window;

    class layout : public window {
        NAO_LOGGER(layout)

        public:
        explicit layout(main_window& w);

        virtual void add_element(window& element) = 0;

        protected:
        /**
         * @note Should be called at the start of overriding implementations
         */
        [[nodiscard]] event_result on_resize(resize_event& e) override;

        private:
        friend class main_window;
        void _set_parent(main_window& win) const;
    };
}