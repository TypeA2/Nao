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
#include "list_view.h"

#include <CommCtrl.h>
#include <Uxtheme.h>

nao::ui::list_view::item::item(list_view& parent)
    : _parent{ &parent } {

}

nao::ui::list_view::item::item(list_view& parent, size_t columns)
    : _parent{ &parent }, _text { columns }, _icons{columns}, _data(columns) {
    
}

nao::ui::list_view::item::item(const item& other)
    : _parent{ other._parent }, _text{ other._text }, _icons{ other._icons }, _data{ other._data } {

}

nao::ui::list_view::item& nao::ui::list_view::item::operator=(const item& other) {
    _parent = other._parent;
    _text = other._text;
    _icons = other._icons;
    _data = other._data;

    return *this;
}

nao::ui::list_view::item::item(item&& other) noexcept
    : _parent{ std::exchange(other._parent, nullptr) }
    , _text  { std::move(other._text)   }
    , _icons { std::move(other._icons)  }
    , _data  { std::move(other._data)   } {

}

nao::ui::list_view::item& nao::ui::list_view::item::operator=(item&& other) noexcept {
    _parent = std::exchange(other._parent, nullptr);
    _text = std::move(other._text);
    _icons = std::move(other._icons);
    _data = std::move(other._data);
    
    return *this;
}

void nao::ui::list_view::item::set_columns(size_t count) {
    _text.resize(count);
    _icons.resize(count);
    _data.resize(count);
}

size_t nao::ui::list_view::item::columns() const {
    return _text.size();
}

void nao::ui::list_view::item::set_text(size_t i, std::string_view val) {
    _text[i] = val;
}

std::string_view nao::ui::list_view::item::text(size_t i) const {
    return _text[i];
}

void nao::ui::list_view::item::set_icon(size_t i, nao::ui::icon val) {
    _icons[i] = std::move(val);
}

const nao::ui::icon& nao::ui::list_view::item::icon(size_t i) const {
    return _icons[i];
}

nao::ui::icon& nao::ui::list_view::item::icon(size_t i) {
    return _icons[i];
}

void nao::ui::list_view::item::set_data(size_t i, std::any val) {
    _data[i] = std::move(val);
}

const std::any& nao::ui::list_view::item::data(size_t i) const {
    return _data[i];
}

std::any& nao::ui::list_view::item::data(size_t i) {
    return _data[i];
}

nao::ui::list_view::list_view(window& parent)
    : window {
        {
            .builtin = true,
            .cls = WC_LISTVIEWA,
            .style = WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
            .parent = &parent
        }
    } {

    SetWindowTheme(_handle, L"Explorer", nullptr);
    ListView_SetExtendedListViewStyle(_handle, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    parent.set_window(*this);
}

nao::ui::list_view::list_view(window& parent, item header) : list_view{ parent, std::move(header), {} } {
    
}

nao::ui::list_view::list_view(window& parent, item header, std::span<item> items) : list_view{ parent } {
    _header = std::move(header);
    
    _items.reserve(items.size());
    std::ranges::move(items, std::back_inserter(_items));
}

void nao::ui::list_view::set_column_count(size_t count) {
    if (count < _columns) {
        size_t remove = _columns - count;
        for (size_t i = 0; i < remove; ++i) {
            ListView_DeleteColumn(_handle, _columns - i - 1);
        }
    } else if (count > _columns) {
        size_t add = count - _columns;

        LVCOLUMNW col;
        col.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH | LVCF_MINWIDTH;
        col.cx = this->client_size().w;
        col.cxMin = col.cx;
        col.fmt = LVCFMT_LEFT;
        for (size_t i = 0; i < add; ++i) {
            col.iOrder = _columns + i;
        }
    }

    _columns = count;
}

size_t nao::ui::list_view::column_count() const {
    return _columns;
}
