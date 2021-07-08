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

#include <libnao/ui/event.h>

#include <functional>
#include <vector>

namespace nao::ui {
    class message_loop final {
        public:
        /* Event filter function, return true if the event should be discarded. */
        using event_filter = std::function<bool(event&)>;

        private:
        std::vector<event_filter> _filters;

        public:
        int run();

        void add_filter(event_filter f);
    };
}
