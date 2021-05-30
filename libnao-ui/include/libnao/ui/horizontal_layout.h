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

#include "layout.h"

#include <vector>

namespace nao {
    class horizontal_layout : public layout {
        NAO_LOGGER(horizontal_layout)

        std::vector<window*> _children;
        public:
        using layout::layout;

        void add_element(window& element) override;

        protected:
        event_result on_resize(resize_event& e) override;
        void reposition() override;
    };
}