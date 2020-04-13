#include "preview.h"

#include "resource.h"

#include "nao_controller.h"

#include "main_window.h"
#include "right_window.h"

#include "list_view.h"
#include "push_button.h"
#include "slider.h"
#include "label.h"
#include "seekable_progress_bar.h"
#include "separator.h"
#include "line_edit.h"
#include "direct2d_image_display.h"

#include "dimensions.h"

#include <algorithm>

preview::preview(nao_view& view)
    : ui_element(&view.window()->right())
    , view { view }, controller { view.get_controller() } {

}

preview::preview(nao_view& view, int string)
    : ui_element(&view.window()->right(), string, view.window()->right().dims().rect(), win32::style | SS_SUNKEN)
    , view { view }, controller { view.get_controller() } {
    
}


list_view_preview::list_view_preview(nao_view& view, item_file_handler* handler)
    : preview(view, IDS_LIST_VIEW_PREVIEW)
    , _handler { handler }
    , _list { this, nao_view::list_view_header(), nao_view::shell_image_list() } {

    _list.set_column_alignment(2, list_view::Right);

    auto items = nao_controller::transform_data_to_row(_handler->data());

    std::sort(items.begin(), items.end(), [this](const list_view_row& first, const list_view_row& second) {
        return controller.order_items(
            static_cast<item_data*>(const_cast<void*>(first.data)),
            static_cast<item_data*>(const_cast<void*>(second.data)),
            _selected_column, _sort_order[_selected_column]) == -1;
        });

    for (const auto& [key, order] : _sort_order) {
        if (key == _selected_column) {
            _list.set_sort_arrow(key, (order == ORDER_NORMAL) ? list_view::UpArrow : list_view::DownArrow);
        } else {
            _list.set_sort_arrow(key, list_view::NoArrow);
        }
    }

    for (const auto& [name, type, size, compressed,
        icon, data] : items) {

        _list.add_item({ name, type, size, compressed }, icon, data);
    }

    // Fit columns
    for (int i = 0; i < _list.column_count() - 1; ++i) {
        int64_t min = 0;

        if (i == 0) {
            min = _list.width() / _list.column_count();
        }

        _list.set_column_width(i, LVSCW_AUTOSIZE, min);
    }
}

list_view& list_view_preview::list() {
    return _list;
}

void list_view_preview::wm_size(int, const dimensions& dims) {
    defer_window_pos().move(_list, dims.rect());
}

void list_view_preview::wm_notify(WPARAM, NMHDR* hdr) {
    if (hdr->hwndFrom != _list.handle()) {
        return;
    }

    switch (hdr->code) {
        case LVN_COLUMNCLICK: {
            // Clicked on a column, sort based on this column
            NMLISTVIEW* view = reinterpret_cast<NMLISTVIEW*>(hdr);

            if (view->iItem != -1) {
                break;
            }

            // If a different column was selected, use the default sort
            if (_selected_column != view->iSubItem) {

                _selected_column = static_cast<data_key>(view->iSubItem);
                _sort_order[_selected_column] = nao_view::list_view_default_sort().at(_selected_column);
            } else {
                // Else swap the current selection
                _sort_order[_selected_column] =
                    (_sort_order[_selected_column] == ORDER_NORMAL) ? ORDER_REVERSE : ORDER_NORMAL;
            }

            for (const auto& [key, order] : _sort_order) {
                if (key == _selected_column) {
                    _list.set_sort_arrow(key, (order == ORDER_NORMAL) ? list_view::UpArrow : list_view::DownArrow);
                } else {
                    _list.set_sort_arrow(key, list_view::NoArrow);
                }
            }

            static auto sort_func = [](LPARAM lparam1, LPARAM lparam2, LPARAM info) {
                list_view_preview const* preview = reinterpret_cast<list_view_preview const*>(info);

                return preview->controller.order_items(
                    reinterpret_cast<item_data*>(lparam1), reinterpret_cast<item_data*>(lparam2),
                    preview->_selected_column, preview->_sort_order.at(preview->_selected_column));
            };

            _list.sort(sort_func, this);
            break;
        }

        case NM_DBLCLK: {
            // Double click to open a nested item
            NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(hdr);
            if (item->iItem >= 0) {
                controller.list_view_preview_clicked(CLICK_DOUBLE_ITEM,
                    _list.get_item_data(item->iItem));
            }
            break;
        }

        case NM_RCLICK: {
            NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(hdr);
            ClientToScreen(_list.handle(), &item->ptAction);

            if (item->iItem >= 0) {
                item_data* data = _list.get_item_data<item_data*>(item->iItem);

                if (data) {
                    controller.create_context_menu_preview(data, item->ptAction);
                }
            } else {
                // Clicked outside
                controller.create_context_menu_preview(nullptr, item->ptAction);
            }
        }

        default: break;
    }
}



audio_player_preview::info_pair::info_pair(ui_element* parent, const std::string& label, const std::string& edit)
    : label { parent, label, LABEL_LEFT }, edit { parent, edit } {
    this->edit.set_read_only(true);
    this->edit.set_ex_style(WS_EX_CLIENTEDGE, false);
}

void audio_player_preview::info_pair::move(defer_window_pos& dwp,
    int64_t partial_offset, int64_t partial_width, int64_t info_offset, int64_t index) {

    dwp.move(label, {
            .x = partial_offset,
            .y = info_offset + index * (dims::control_height + dims::gutter_size),
            .width = partial_width / 5,
            .height = dims::control_height })
        .move(edit, {
            .x = partial_offset + (partial_width / 5),
            .y = info_offset + index * (dims::control_height + dims::gutter_size) - 1,
            .width = 4 * (partial_width / 5),
            .height = dims::control_height });
}


audio_player_preview::audio_player_preview(nao_view& view, std::unique_ptr<audio_player> player)
    : preview(view, IDS_AUDIO_PLAYER_PREVIEW)
    , _player { std::move(player) }
    , _progress_bar { this, 0, 1000 }
    , _toggle_button { this, win32::icon{} }
    , _volume_slider { this, 0, 100 }
    , _volume_display { this, "", LABEL_CENTER }
    , _progress_display { this, "", LABEL_LEFT }
    , _duration_display { this, "", LABEL_RIGHT }

    , _separator { this, SEPARATOR_HORIZONTAL }

    , _codec { this, "Codec:", _player->provider()->name() }
    , _rate { this, "Sample rate:", std::to_string(_player->provider()->rate()) + " Hz" }
    , _channels { this, "Channels:", std::to_string(_player->provider()->channels()) }
    , _type { this, "Sample type:", "" } {

    win32::dynamic_library mmcndmgr("mmcndmgr.dll");

    _play_icon = mmcndmgr.load_icon_scaled(30529, { dims::play_button_size, dims::play_button_size });
    _pause_icon = mmcndmgr.load_icon_scaled(30531, { dims::play_button_size, dims::play_button_size });

    _toggle_button.set_icon(_play_icon);
    _volume_slider.set_position(100);

    int64_t volume = static_cast<int64_t>(round(_player->volume_log() * 100.));
    _volume_slider.set_position(volume);
    _volume_display.set_text(std::to_string(volume) + "%");

    _duration = _player->duration();
    _duration_display.set_text(utils::format_minutes(_duration, false));

    _volume_display_size = _volume_display.text_extent_point();
    _duration_size = _duration_display.text_extent_point();
    _progress_size = _progress_display.text_extent_point();

    _set_progress(std::chrono::nanoseconds(0));

    _player->add_event(EVENT_START, [this] {
        _toggle_button.set_icon(_pause_icon);
        SetTimer(handle(), 0, 100, nullptr);
        });
    _player->add_event(EVENT_STOP, [this] {
        _toggle_button.set_icon(_play_icon);
        KillTimer(handle(), 0);
        });

    auto provider = _player->provider();

    _type.edit.set_text(samples::format_name(provider->format()));

    audio_player_preview::wm_size(0, dims());
}

void audio_player_preview::wm_size(int, const dimensions& dims) {
    auto [width, height] = dims;

    // Use 70% of width for full-width controls
    int64_t partial_width = static_cast<int64_t>(width * 0.7);
    int64_t partial_offset = static_cast<int64_t>(width * 0.15);

    int64_t controls_height = 3 * dims::control_height + dims::play_button_size + dims::volume_slider_height + 6 * dims::gutter_size;
    int64_t info_offset = controls_height + 2 + (dims::control_height / 2) + dims::gutter_size;

    defer_window_pos dwp;

    dwp
        .move(_progress_bar, {
                .x = partial_offset,
                .y = dims::gutter_size,
                .width = partial_width,
                .height = dims::control_height })
        .move(_toggle_button, {
                .x = (width / 2) - (dims::play_button_size / 2),
                .y = 2 * dims::control_height + 3 * dims::gutter_size,
                .width = dims::play_button_size,
                .height = dims::play_button_size })
        .move(_volume_slider, {
                .x = (width / 2) - (dims::volume_slider_width / 2),
                .y = 2 * dims::control_height + dims::play_button_size + 4 * dims::gutter_size,
                .width = dims::volume_slider_width,
                .height = dims::volume_slider_height })
        .move(_volume_display, {
                .x = (width / 2) - (_volume_display_size.width / 2),
                .y = 2 * dims::control_height + dims::play_button_size + dims::volume_slider_height + 5 * dims::gutter_size,
                .width = _volume_display_size.width,
                .height = _volume_display_size.height })
        .move(_progress_display, {
                .x = partial_offset,
                .y = dims::control_height + 2 * dims::gutter_size,
                .width = _progress_size.width,
                .height = _progress_size.height })
        .move(_duration_display, {
                .x = partial_offset + (partial_width - _duration_size.width),
                .y = dims::control_height + 2 * dims::gutter_size,
                .width = _duration_size.width,
                .height = _duration_size.height })

        .move(_separator, {
                .x = partial_offset,
                .y = controls_height,
                .width = partial_width,
                .height = 2 });


    _codec.move(dwp, partial_offset, partial_width, info_offset, 0);
    _rate.move(dwp, partial_offset, partial_width, info_offset, 1);
    _channels.move(dwp, partial_offset, partial_width, info_offset, 2);
    _type.move(dwp, partial_offset, partial_width, info_offset, 3);
}

void audio_player_preview::wm_command(WORD id, WORD code, HWND target) {
    if (target == _toggle_button.handle()) {
        if (_player->paused()) {
            if (_player->eof()) {
                _player->reset();
            }

            _player->play();
            _toggle_button.set_icon(_pause_icon);
        } else {
            _player->pause();
            _toggle_button.set_icon(_play_icon);
        }
    }
}


LRESULT audio_player_preview::wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_HSCROLL: {
            switch (LOWORD(wparam)) {
                case SB_THUMBPOSITION:
                case SB_THUMBTRACK:
                case SB_LINERIGHT:
                case SB_LINELEFT:
                case SB_PAGERIGHT:
                case SB_PAGELEFT:
                case SB_RIGHT:
                case SB_LEFT: {
                    // Slider moved
                    int64_t new_pos = _volume_slider.get_position();
                    _player->set_volume_log(new_pos / 100.f);
                    _volume_display.set_text(std::to_string(new_pos) + "%");
                    break;
                }

                default: break;
            }
            break;
        }

        case WM_CTLCOLORSTATIC:
            return COLOR_WINDOW + 1;

        case WM_TIMER:
            if (wparam == 0) {
                auto current = _player->pos();
                _set_progress(current);
                _progress_bar.set_progress(
                    static_cast<uintmax_t>(round((current.count() / static_cast<double>(_duration.count())) * 1000)));

            }
            break;

        case PB_CAPTURE:
            // If audio is playing, pause it
            if (!_player->paused()) {
                _resume_after_seek = true;
                _player->pause();
            } else {
                _resume_after_seek  = false;
            }
            break;

        case PB_SEEK:
        case PB_RELEASE: {
            double promille = wparam / 1000.;
            auto progress_ns = std::chrono::nanoseconds(
                static_cast<std::chrono::nanoseconds::rep>(round(promille * _duration.count())));

            if (msg == PB_SEEK) {
                _set_progress(progress_ns);
            } else {
                _player->seek(progress_ns);
                if (_resume_after_seek) {
                    _player->play();
                }
            }

            break;
        }

        default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return 0;
}

void audio_player_preview::_set_progress(std::chrono::nanoseconds progress) {
    _progress_display.set_text(utils::format_minutes(progress, false));

    _progress_size = _progress_display.text_extent_point();
}



image_viewer_preview::image_viewer_preview(nao_view& view, const image_data& data)
    : preview(view, IDS_IMAGE_PREVIEW)
    , _window { this, (data.format() == PIXEL_BGRA32) ? data.data() : data.as(PIXEL_BGRA32).data(), data.dims() } {

}

void image_viewer_preview::wm_size(int, const dimensions& dims) {
    defer_window_pos().move(_window, { 0, 0, dims.width, dims.height - 150 });
}


rectangle video_player_preview::player_canvas::start_rect(ui_element* parent) {
    rectangle p = parent->rect();
    return {
        .x = p.x,
        .y = p.x,
        .width = p.width,
        .height = p.height - std::min(display_margin, parent->height() / 3)
    };
}

video_player_preview::player_canvas::player_canvas(ui_element* parent)
    : ui_element(parent, IDS_VIDEO_PLAYER_PREVIEW_CANVAS, start_rect(parent), WS_CHILD | WS_VISIBLE) {
    
}


video_player_preview::video_player_preview(nao_view& view, av_file_handler* handler)
    : preview(view, IDS_VIDEO_PLAYER_PREVIEW)
    , _player_display { this }
    , _player { handler->get_stream(), handler->get_path(), this, &_player_display }
    , _toggle_button { this, win32::icon{} } {

    win32::dynamic_library mmcndmgr("mmcndmgr.dll");

    _play_icon = mmcndmgr.load_icon_scaled(30529, { dims::play_button_size, dims::play_button_size });
    _pause_icon = mmcndmgr.load_icon_scaled(30531, { dims::play_button_size, dims::play_button_size });

    _toggle_button.set_icon(_play_icon);

    video_player_preview::wm_size(0, dims());
    ui_element::wm_paint();
}

void video_player_preview::wm_paint() {
    _player.repaint();
}

void video_player_preview::wm_size(int, const dimensions& dims) {
    int64_t actual_margin = std::min(display_margin, dims.height / 3);

    defer_window_pos()
        .move(_player_display, {
            .x = 0,
            .y = 0,
            .width = dims.width,
            .height = dims.height - actual_margin })

        .move(_toggle_button, {
            .x = dims::gutter_size,
            .y = (dims.height - actual_margin) + dims::gutter_size,
            .width = dims::play_button_size,
            .height = dims::play_button_size });

    _player.set_position({ 0, 0, dims.width, dims.height - actual_margin });
}

LRESULT video_player_preview::wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == mf::player::WM_APP_PLAYER_EVENT) {
        _player.handle_event(mf::media_event { reinterpret_cast<IMFMediaEvent*>(wparam), false });
        return 0;
    }

    return preview::wnd_proc(hwnd, msg, wparam, lparam);
}
