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

#include <libnao/util/encoding.h>

#include <CommCtrl.h>
#include <Uxtheme.h>

void nao::ui::list_view::item::set_parent(list_view* new_parent) {
    _parent = new_parent;
}

/*
nao::ui::list_view::item::item(list_view& parent) {
    set_parent(&parent);
}

nao::ui::list_view::item::item(list_view& parent, size_t columns)
    : _text { columns }, _icons{columns}, _data(columns) {
    set_parent(&parent);
}*/

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

nao::ui::list_view::list_view(window& parent, item_ptr header) : list_view{ parent, std::move(header), {} } {
    
}

nao::ui::list_view::list_view(window& parent, item_ptr header, std::span<item_ptr> items) : list_view{ parent } {
    _header = std::move(header);
    _header_changed();
    
    _items.reserve(items.size());
    for (item_ptr& new_item : items) {
        add_item(std::move(new_item));
    }
}

void nao::ui::list_view::set_header(item_ptr new_header) {
    _header = std::move(new_header);

    _header_changed();
}

void nao::ui::list_view::add_item(item_ptr new_item) {
    if (std::ranges::find(_items, new_item.get(), &item_ptr::get) == _items.end()) {
        item_ptr& inserted = _items.emplace_back(std::move(new_item));
        inserted->set_parent(this);
    }
}

bool nao::ui::list_view::has_item(item* needle) const {
    return (std::ranges::find(_items, needle, &item_ptr::get) != _items.end());
}


void nao::ui::list_view::_header_changed() {
    size_t columns = _header->columns();

    for (size_t i = 0; i < columns; ++i) {
        ListView_DeleteColumn(_handle, i);
    }

    LVCOLUMNW col;
    col.mask = LVCF_TEXT | LVCF_FMT | LVCF_ORDER | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;

    for (size_t i = 0; i < columns; ++i) {
        col.iOrder = static_cast<int>(columns + i);

        std::wstring wide_col = nao::utf8_to_wide(_header->text(i));
        col.pszText = wide_col.data();

        col.cx = static_cast<int>(15 + _string_width(wide_col));

        ListView_InsertColumn(_handle, col.iOrder, &col);
    }

}

size_t nao::ui::list_view::_string_width(const std::wstring& string) {
    return SendMessageW(_handle, LVM_GETSTRINGWIDTHW, 0, reinterpret_cast<LPARAM>(string.c_str()));
}
