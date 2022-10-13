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

#include "icon.h"

#include <libnao/util/win32.h>
#include <libnao/util/event_handler.h>

#include <any>
#include <span>
#include <functional>

namespace nao::ui {
    class list_view : public window {
        NAO_LOGGER(list_view);

        public:
        class item {
            NAO_LOGGER(list_view::item);
            list_view* _parent;
            std::vector<std::string> _text;
            std::vector<icon> _icons;
            std::vector<std::any> _data;

            public:
            explicit item(list_view& parent);
            item(list_view& parent, size_t columns);

            item(const item& other);
            item& operator=(const item& other);

            item(item&& other) noexcept;
            item& operator=(item&& other) noexcept;

            void set_columns(size_t count);
            [[nodiscard]] size_t columns() const;

            void set_text(size_t i, std::string_view val);
            [[nodiscard]] std::string_view text(size_t i) const;

            void set_icon(size_t i, nao::ui::icon val);
            [[nodiscard]] const nao::ui::icon& icon(size_t i) const;
            [[nodiscard]] nao::ui::icon& icon(size_t i);

            void set_data(size_t i, std::any val);
            [[nodiscard]] const std::any& data(size_t i) const;
            [[nodiscard]] std::any& data(size_t i);
        };

        private:

        size_t _columns {};

        item _header{ *this };
        std::vector<item> _items;

        public:
        explicit list_view(window& parent);
        list_view(window& parent, item header);
        list_view(window& parent, item header, std::span<item> items);

        void set_column_count(size_t count);
        [[nodiscard]] size_t column_count() const;
    };
}
