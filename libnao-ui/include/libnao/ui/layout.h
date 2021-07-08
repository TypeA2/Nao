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

#include <libnao/util/logging.h>

namespace nao::ui {
    class main_window;

    class layout_item {
        window& _child;

        public:
        virtual ~layout_item() = default;

        layout_item() = delete;
        explicit layout_item(window& win);

        layout_item(layout_item&&) noexcept = default;
        layout_item& operator=(layout_item&&) noexcept = delete;

        layout_item(const layout_item&) = delete;
        layout_item& operator=(const layout_item&) = delete;

        [[nodiscard]] window& item() const;
    };

    class layout : public window {
        NAO_LOGGER(layout);

        margins _content_margins{};
        long _content_spacing{};

        protected:
        std::vector<std::unique_ptr<layout_item>> children;

        public:
        /* Constant to signify automatic sizing */
        static constexpr long fill = -1;

        explicit layout(window& w);

        void add_element(window& element);

        /**
         * Determines the margins at the edge of the layout.
         */
        void set_content_margins(const margins& margins);
        void set_content_margins(long top, long right, long bot, long left);
        [[nodiscard]] margins content_margins() const;

        /**
         * Determines the spacing between elements of the layout
         */
        void set_content_spacing(long spacing);
        [[nodiscard]] long content_spacing() const;

        void set_window(window& w) override;

        protected:
        /**
         * Layout-specific message handling like forwarding to children
         */
        [[nodiscard]] event_result on_event(event& e) override;

        /**
         * @note Should be called at the start of overriding implementations
         */
        [[nodiscard]] event_result on_resize(resize_event& e) override;

        /**
         * Re-calculates child positions
         */
        virtual void reposition();

        /**
         * Retrieve a layout_item to be inserted into the children list
         */
        [[nodiscard]] virtual std::unique_ptr<layout_item> make_item(window& element);
    };
}