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

namespace nao::ui {
    enum class layout_direction {
        horizontal,
        vertical,
    };

    class directional_layout : public layout {
        NAO_LOGGER(horizontal_layout);

        layout_direction _direction;

        public:
        directional_layout(window& parent, layout_direction dir);

        protected:
        void reposition() override;

        private:
        void _reposition_horizontal();
        void _reposition_vertical();
    };

    class horizontal_layout : public directional_layout {
        public:
        explicit horizontal_layout(window& parent);
    };

    class vertical_layout : public directional_layout {
        public:
        explicit vertical_layout(window& parent);
    };
}