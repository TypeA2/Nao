/*  This file is part of libnao-ui.

    libnao-ui is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao-ui is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libnao-ui.  If not, see <https://www.gnu.org/licenses/>.   */

#pragma once

#include <nao/string_view.h>

#include "nao/widget.h"

namespace nao::ui {
    class NAOUI_API push_button : public widget {
        NAO_DEFINE_PTR(NAOUI_API, button_private, _d);

        public:
        /**
         * @brief Construct without any text
         * @param parent
         */
        push_button(widget* parent = nullptr);

        /**
         * @brief Construct with a givent parent element and text
         * @param text - The text to display inside the button
         * @param parent
         */
        push_button(string_view text, widget* parent = nullptr);
    };
}
