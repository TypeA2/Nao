#include "preview.h"

#include "resource.h"

#include "nao_view.h"
#include "nao_controller.h"

#include "main_window.h"
#include "right_window.h"

#include "list_view.h"
#include "push_button.h"
#include "slider.h"
#include "label.h"
#include "seekable_progress_bar.h"

#include "audio_player.h"

#include "utils.h"
#include "dimensions.h"
#include "dynamic_library.h"

#include <algorithm>

preview::preview(nao_view& view, item_provider* provider)
    : ui_element(view.window()->right())
    , view(view), controller(view.controller), provider(provider) {

}

preview::~preview() { }

list_view_preview::list_view_preview(nao_view& view, item_provider* provider) : preview(view, provider)
    , _m_sort_order(nao_view::list_view_default_sort()), _m_selected_column(KEY_NAME) {

    std::wstring class_name =  register_once(IDS_LIST_VIEW_PREVIEW);

    auto [x, y, width, height] = parent()->dimensions();

    HWND handle = create_window(class_name, L"", WS_CHILD | WS_VISIBLE | SS_SUNKEN,
        { 0, 0, width, height }, parent(),
        new wnd_init(this, &list_view_preview::_wnd_proc));

    ASSERT(handle);
}

bool list_view_preview::wm_create(CREATESTRUCTW*) {
    _m_list = std::make_unique<list_view>(this, nao_view::list_view_header(), nao_view::shell_image_list());
    _m_list->set_column_alignment(2, list_view::Right);

    auto items = nao_controller::transform_data_to_row(provider->data());

    std::sort(items.begin(), items.end(), [this](const list_view_row& first, const list_view_row& second) {
        return controller.order_items(
            static_cast<item_data*>(const_cast<void*>(first.data)),
            static_cast<item_data*>(const_cast<void*>(second.data)),
            _m_selected_column, _m_sort_order[_m_selected_column]) == -1;
        });

    for (const auto& [key, order] : _m_sort_order) {
        if (key == _m_selected_column) {
            _m_list->set_sort_arrow(key, (order == ORDER_NORMAL) ? list_view::UpArrow : list_view::DownArrow);
        } else {
            _m_list->set_sort_arrow(key, list_view::NoArrow);
        }
    }

    for (const auto& [name, type, size, compressed,
        icon, data] : items) {

        _m_list->add_item({ name, type, size, compressed }, icon, data);
    }

    // Fit columns
    for (int i = 0; i < _m_list->column_count() - 1; ++i) {
        int min = 0;

        if (i == 0) {
            min = _m_list->width() / _m_list->column_count();
        }

        _m_list->set_column_width(i, LVSCW_AUTOSIZE, min);
    }

    return true;
}

void list_view_preview::wm_size(int, int width, int height) {
    
    defer_window_pos()
        .move(_m_list, { 0, 0, width, height });
}

LRESULT list_view_preview::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_NOTIFY: {
            NMHDR* nm = reinterpret_cast<NMHDR*>(lparam);

            if (nm->hwndFrom == _m_list->handle()) {
                switch (nm->code) {
                    case LVN_COLUMNCLICK: {
                        // Clicked on a column, sort based on this column
                        NMLISTVIEW* view = reinterpret_cast<NMLISTVIEW*>(nm);

                        if (view->iItem != -1) {
                            break;
                        }

                        // If a different column was selected, use the default sort
                        if (_m_selected_column != view->iSubItem) {

                            _m_selected_column = static_cast<data_key>(view->iSubItem);
                            _m_sort_order[_m_selected_column] = nao_view::list_view_default_sort().at(_m_selected_column);
                        } else {
                            // Else swap the current selection
                            _m_sort_order[_m_selected_column] =
                                (_m_sort_order[_m_selected_column] == ORDER_NORMAL) ? ORDER_REVERSE : ORDER_NORMAL;
                        }

                        for (const auto& [key, order] : _m_sort_order) {
                            if (key == _m_selected_column) {
                                _m_list->set_sort_arrow(key, (order == ORDER_NORMAL) ? list_view::UpArrow : list_view::DownArrow);
                            } else {
                                _m_list->set_sort_arrow(key, list_view::NoArrow);
                            }
                        }

                        static auto sort_func = [](LPARAM lparam1, LPARAM lparam2, LPARAM info) {
                            list_view_preview const* preview = reinterpret_cast<list_view_preview const*>(info);

                            return preview->controller.order_items(
                                reinterpret_cast<item_data*>(lparam1), reinterpret_cast<item_data*>(lparam2),
                                preview->_m_selected_column, preview->_m_sort_order.at(preview->_m_selected_column));
                        };

                        _m_list->sort(sort_func, this);
                        break;
                    }

                    case NM_DBLCLK: {
                        // Double click to open a nested item
                        NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(nm);
                        if (item->iItem >= 0) {
                            controller.list_view_preview_clicked(CLICK_DOUBLE_ITEM,
                                _m_list->get_item_data(item->iItem));
                        }
                        break;
                    }

                    case NM_RCLICK: {
                        NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(nm);
                        ClientToScreen(_m_list->handle(), &item->ptAction);

                        if (item->iItem >= 0) {
                            item_data* data = _m_list->get_item_data<item_data*>(item->iItem);

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

            break;
        }

        default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return 0;
}





audio_player_preview::audio_player_preview(nao_view& view, item_provider* provider) : preview(view, provider)
    , _m_duration { 0 }
    , _m_volume_display_size { }, _m_progress_size {  }, _m_duration_size { }
    , _m_resume_after_seek { false } {

    std::wstring class_name = register_once(IDS_AUDIO_PLAYER_PREVIEW);

    auto [x, y, width, height] = parent()->dimensions();

    HWND handle = create_window(class_name, L"", WS_CHILD | WS_VISIBLE | SS_SUNKEN,
        { 0, 0, width, height }, parent(),
        new wnd_init(this, &audio_player_preview::_wnd_proc));

    ASSERT(handle);
}

bool audio_player_preview::wm_create(CREATESTRUCTW*) {
    dynamic_library mmcndmgr("mmcndmgr.dll");

    _m_play_icon = mmcndmgr.load_icon_scaled(30529, dims::play_button_size, dims::play_button_size);
    _m_pause_icon = mmcndmgr.load_icon_scaled(30531, dims::play_button_size, dims::play_button_size);

    _m_toggle_button = std::make_unique<push_button>(this, _m_play_icon);
    _m_volume_slider = std::make_unique<slider>(this, 0, 100);
    _m_volume_slider->set_position(100);

    _m_volume_display = std::make_unique<label>(this, "", LABEL_CENTER);
    _m_progress_display = std::make_unique<label>(this, "", LABEL_LEFT);
    _m_duration_display = std::make_unique<label>(this, "", LABEL_RIGHT);

    _m_player = std::make_unique<audio_player>(provider->get_stream(), provider->get_path(), controller);

    int64_t volume = static_cast<int64_t>(round(_m_player->get_volume_log() * 100.));
    _m_volume_slider->set_position(volume);
    _m_volume_display->set_text(std::to_string(volume) + "%");

    _m_duration = _m_player->get_duration();
    _m_duration_display->set_text(utils::format_minutes(_m_duration, false));

    _m_progress_bar = std::make_unique<seekable_progress_bar>(this, 0, 1000);

    _m_volume_display_size = _m_volume_display->text_extent_point();
    _m_duration_size = _m_duration_display->text_extent_point();
    _m_progress_size = _m_progress_display->text_extent_point();

    _set_progress(std::chrono::nanoseconds(0));

    std::function<void()> timer_start_func = [this] {
        SetTimer(handle(), 0, 100, nullptr);
    };

    std::function<void()> timer_stop_func = [this] {
        KillTimer(handle(), 0);
    };

    _m_player->add_event(EVENT_START, timer_start_func);
    _m_player->add_event(EVENT_RESTART, timer_start_func);

    _m_player->add_event(EVENT_PAUSE, timer_stop_func);
    _m_player->add_event(EVENT_STOP, timer_stop_func);
    _m_player->add_event(EVENT_STOP, [this] {
        _m_toggle_button->set_icon(_m_play_icon);
        });

    return true;
}

void audio_player_preview::wm_size(int, int width, int height) {
    // Use 70% of width for full-width controls
    long partial_width = static_cast<long>(width * 0.7);
    long partial_offset = static_cast<long>(width * 0.15);

    defer_window_pos()
        .move(_m_progress_bar, {
                .x = partial_offset,
                .y = dims::gutter_size,
                .width = partial_width,
                .height = dims::control_height })

        .move(_m_toggle_button, {
                .x = (width / 2) - (dims::play_button_size / 2),
                .y = 2 * dims::control_height + 3 * dims::gutter_size,
                .width = dims::play_button_size,
                .height = dims::play_button_size })

        .move(_m_volume_slider, {
                .x = (width / 2) - (dims::volume_slider_width / 2),
                .y = 2 * dims::control_height + dims::play_button_size + 4 * dims::gutter_size,
                .width = dims::volume_slider_width,
                .height = dims::volume_slider_height })

        .move(_m_volume_display, {
                .x = (width / 2) - (_m_volume_display_size.width / 2),
                .y = 2 * dims::control_height + dims::play_button_size + dims::volume_slider_height + 5 * dims::gutter_size,
                .width = _m_volume_display_size.width,
                .height = _m_volume_display_size.height })

        .move(_m_progress_display, {
                .x = partial_offset,
                .y = dims::control_height + 2 * dims::gutter_size,
                .width = _m_progress_size.width,
                .height = _m_progress_size.height })

        .move(_m_duration_display, {
                .x = partial_offset + (partial_width - _m_duration_size.width),
                .y = dims::control_height + 2 * dims::gutter_size,
                .width = _m_duration_size.width,
                .height = _m_duration_size.height
            });
}

LRESULT audio_player_preview::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_COMMAND: {
            HWND target = reinterpret_cast<HWND>(lparam);

            if (target == _m_toggle_button->handle()) {
                _m_player->toggle_playback();

                switch (_m_player->state()) {
                    case STATE_PAUSED:
                        _m_toggle_button->set_icon(_m_play_icon);
                        break;

                    case STATE_PLAYING:
                        _m_toggle_button->set_icon(_m_pause_icon);
                        break;

                    default: break;
                }
            }

            break;
        }

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
                    int64_t new_pos = _m_volume_slider->get_position();
                    _m_player->set_volume_log(new_pos / 100.f);
                    _m_volume_display->set_text(std::to_string(new_pos) + "%");
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
                auto current = _m_player->get_current_time();
                _set_progress(current);
                _m_progress_bar->set_progress(
                    static_cast<uintmax_t>(round((current.count() / static_cast<double>(_m_duration.count())) * 1000)));

            }
            break;

        case PB_CAPTURE:
            // If audio is playing, pause it
            if (_m_player->state() == STATE_PLAYING) {
                _m_resume_after_seek = true;
                _m_player->toggle_playback();
            } else {
                _m_resume_after_seek  = false;
            }
            break;

        case PB_SEEK:
        case PB_RELEASE: {
            double promille = wparam / 1000.;
            auto progress_ns = static_cast<std::chrono::nanoseconds::rep>(round(promille * _m_duration.count()));

            if (msg == PB_SEEK) {
                _set_progress(std::chrono::nanoseconds(progress_ns));
            } else {
                _m_player->seek(std::chrono::nanoseconds(progress_ns), _m_resume_after_seek);
            }

            break;
        }

        default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return 0;
}

void audio_player_preview::_set_progress(std::chrono::nanoseconds progress) {
    _m_progress_display->set_text(utils::format_minutes(progress, false));

    _m_progress_size = _m_progress_display->text_extent_point();
}
