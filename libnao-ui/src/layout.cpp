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

#include "layout.h"

#include "main_window.h"

nao::ui::layout_item::layout_item(window& win) : _child{ win } {
    
}

nao::ui::window& nao::ui::layout_item::item() const {
    return _child;
}

nao::ui::layout::layout(window& w) : window{
        {
            .cls = "nao_layout",
            .style = WS_VISIBLE | WS_CHILD,
            .pos = { 0, 0 },
            .parent = &w
        } } {
    w.set_window(*this);
}

void nao::ui::layout::add_element(window& element) {
    children.emplace_back(make_item(element));
    element.set_parent(*this);
}

void nao::ui::layout::set_content_margins(const margins& margins) {
    assert(margins.top >= 0 && margins.right >= 0 && margins.bot >= 0 && margins.left >= 0);
    _content_margins = margins;
    reposition();
}

void nao::ui::layout::set_content_margins(long top, long right, long bot, long left) {
    set_content_margins({ top, right, bot, left });
}


nao::margins nao::ui::layout::content_margins() const {
    return _content_margins;
}

void nao::ui::layout::set_content_spacing(long spacing) {
    assert(spacing >= 0);
    _content_spacing = spacing;
    reposition();
}

long nao::ui::layout::content_spacing() const {
    return _content_spacing;
}

void nao::ui::layout::set_window(window& w) {
    add_element(w);
}

nao::ui::event_result nao::ui::layout::on_event(event& e) {
    const event::native_event& native = e.native();

    // Command sent to child of this window
    if (auto child = reinterpret_cast<HWND>(native.lparam);
        native.msg == WM_COMMAND
        && GetParent(child) == _handle) {
        
        auto it = std::ranges::find_if(children, [&](auto& item) {
            return item->item().handle() == child;
        });

        if (it != children.end()) {
            return send_event((*it)->item(), e);
        }
    }

    return window::on_event(e);
}


nao::ui::event_result nao::ui::layout::on_resize(resize_event& e) {
    reposition();
    return event_result::ok;
}

void nao::ui::layout::reposition() {
    // Need an empty implementation for when a resize event is received during construction
}

std::unique_ptr<nao::ui::layout_item> nao::ui::layout::make_item(window& element) {
    return std::make_unique<layout_item>(element);
}
