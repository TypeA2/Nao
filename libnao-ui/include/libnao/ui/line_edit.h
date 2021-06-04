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

#include <libnao/util/win32.h>

namespace nao {
    class line_edit : public window {
        NAO_LOGGER(line_edit)

        win32::gdi_object _font;
        std::string _text;

        public:
        explicit line_edit(window& parent);
        line_edit(window& parent, std::string_view text);

        void set_text(std::string_view text);
        [[nodiscard]] std::string_view text() const;

        protected:
        [[nodiscard]] event_result on_event(event& e) override;
    };
}
