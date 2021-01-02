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

#include "push_button.h"

#include "layout.h"

namespace nao {
    push_button::push_button(std::string_view text, layout& parent) : window{
        {
            .builtin = true,
            .cls = "BUTTON",
            .style = WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            .name = text,
            .pos = { 50, 50 },
            .size = { 50, 50 },
            .parent = &parent,
        }} {

        parent.add_element(*this);
    }
}
