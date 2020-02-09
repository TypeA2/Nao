#include "preview.h"

#include "resource.h"

#include "nao_view.h"
#include "nao_controller.h"

#include "main_window.h"
#include "right_window.h"

#include "list_view.h"
#include "push_button.h"
#include "slider.h"
#include "line_edit.h"

#include "audio_player.h"

#include "utils.h"
#include "dimensions.h"
#include "dynamic_library.h"

#include <algorithm>
#include <unordered_set>


preview::preview(nao_view& view, item_provider* provider)
    : ui_element(view.window()->right())
    , view(view), controller(view.controller), provider(provider) {

}

preview::~preview() { }

std::wstring preview::register_once(int id) {
    static std::unordered_set<int> registered_classes;

    std::wstring class_name = load_wstring(id);

    if (!registered_classes.contains(id)) {
        WNDCLASSEXW wcx {
            .cbSize = sizeof(WNDCLASSEXW),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = wnd_proc_fwd,
            .hInstance = instance(),
            .hCursor = LoadCursorW(nullptr, IDC_ARROW),
            .hbrBackground = HBRUSH(COLOR_WINDOW + 1),
            .lpszClassName = class_name.c_str()
        };

        ASSERT(RegisterClassExW(&wcx) != 0);

        registered_classes.insert(id);
    }

    return class_name;
}

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






audio_player_preview::audio_player_preview(nao_view& view, item_provider* provider) : preview(view, provider) {
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

    _m_volume_display = std::make_unique<line_edit>(this, "100%");
    _m_volume_display->set_style(WS_DISABLED);

    _m_player = std::make_unique<audio_player>(provider->get_stream(), provider->get_path(), controller);

    return true;
}

void audio_player_preview::wm_size(int, int width, int height) {
    defer_window_pos()
        .move(_m_toggle_button, { ((width - 2 * dims::gutter_size) / 2) - (dims::play_button_size / 2),
            dims::gutter_size, dims::play_button_size, dims::play_button_size })
        .move(_m_volume_slider, { width - dims::volume_slider_width - dims::gutter_size,
            dims::gutter_size, dims::volume_slider_width, dims::play_button_size })
        .move(_m_volume_display, { width - dims::volume_slider_width - (dims::gutter_size * 2) - dims::volume_display_width,
            dims::gutter_size, dims::volume_display_width, dims::control_height });
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
                    _m_player->set_volume_percent(new_pos / 100.f);
                    _m_volume_display->set_text(std::to_string(new_pos) + "%");
                    break;
                }

                default: break;
            }
            break;
        }

        case WM_CTLCOLORSTATIC:
            return COLOR_WINDOW + 1;

        default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return 0;
}
