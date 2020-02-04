#include "nao_view.h"

#include "nao_controller.h"

#include "main_window.h"
#include "left_window.h"
#include "right_window.h"

#include "line_edit.h"
#include "push_button.h"
#include "list_view.h"

#include "utils.h"
#include "resource.h"

const std::vector<std::string>& nao_view::list_view_header() {
    static std::vector<std::string> vec { "Name", "Type", "Size", "Compressed" };

    return vec;
}

const std::map<data_key, sort_order>& nao_view::list_view_default_sort() {
    static std::map<data_key, sort_order> map {
        { KEY_NAME, ORDER_NORMAL },
        { KEY_TYPE, ORDER_NORMAL },
        { KEY_SIZE, ORDER_REVERSE },
        { KEY_COMP, ORDER_REVERSE }
    };

    return map;
}

IImageList* nao_view::shell_image_list() {
    static IImageList* imglist = nullptr;

    if (!imglist) {
        if (FAILED(SHGetImageList(SHIL_SMALL, IID_PPV_ARGS(&imglist)))) {
            utils::coutln("failed to retrieve main image list");
            return nullptr;
        }

        return imglist;
    }

    // Increment ref count if already initialised
    imglist->AddRef();

    return imglist;
}

nao_view::nao_view(nao_controller& controller) : controller(controller)
    , _m_sort_order(list_view_default_sort()), _m_selected_column(KEY_NAME) {

}

nao_view::~nao_view() {
    (void) this;
}

void nao_view::setup() {
    _m_main_window = std::make_unique<main_window>(this);
}

void nao_view::set_path(const std::string& path) const {
    auto left = _m_main_window->left();

    // Display the current path in the line edit above
    left->path()->set_text(path);

    // Only enable the up button whenever we are not at the root path
    left->view_up()->set_enabled(path != "\\");
}

void nao_view::clear_view(const std::function<void(void*)>& deleter) const {
    _m_main_window->left()->list()->clear(deleter);
}

void nao_view::fill_view(std::vector<list_view_row> items) const {
    if (items.empty()) {
        return;
    }

    list_view* list = _m_main_window->left()->list();

    std::sort(items.begin(), items.end(), [this](const list_view_row& first, const list_view_row& second) {
        return controller.order_items(
            static_cast<item_data*>(const_cast<void*>(first.data)),
            static_cast<item_data*>(const_cast<void*>(second.data)),
            _m_selected_column, selected_column_order()) == -1;
        });

    for (const auto& [key, order] : _m_sort_order) {
        if (key == _m_selected_column) {
            list->set_sort_arrow(key, (order == ORDER_NORMAL) ? list_view::UpArrow : list_view::DownArrow);
        } else {
            list->set_sort_arrow(key, list_view::NoArrow);
        }
    }

    for (const auto& [name, type, size, compressed,
            icon ,data] : items) {

        list->add_item({ name, type, size, compressed }, icon, data);
    }

    // Fit columns
    for (int i = 0; i < list->column_count() - 1; ++i) {
        int min = 0;

        if (i == 0) {
            min = list->width() / list->column_count();
        }

        list->set_column_width(i, LVSCW_AUTOSIZE, min);
    }
}

void nao_view::button_clicked(view_button_type which) const {
    click_event type = CLICK_FIRST;

    switch (which) {
        case BUTTON_UP:
            type = CLICK_MOVE_UP;
            break;
    }

    controller.clicked(type);
}

void nao_view::list_clicked(NMHDR* nm) {
    switch (nm->code) {
        case NM_DBLCLK: {
            // Double-click, opened an item
            NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(nm);

            if (item->iItem >= 0) {
                controller.clicked(CLICK_DOUBLE_ITEM,
                    _m_main_window->left()->list()->get_item_data(item->iItem));
            }
            break;
        }

        case NM_CLICK: {
            // Single left-click, preview item (if possible)
            NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(nm);
            if (item->iItem >= 0) {
                controller.clicked(CLICK_SINGLE_ITEM,
                    _m_main_window->left()->list()->get_item_data(item->iItem));
            }
            break;
        }

        case LVN_COLUMNCLICK: {
            // Clicked on a column, sort based on this column
            NMLISTVIEW* view = reinterpret_cast<NMLISTVIEW*>(nm);
            list_view* list = _m_main_window->left()->list();

            if (view->iItem != -1) {
                break;
            }

            // If a different column was selected, use the default sort
            if (_m_selected_column != view->iSubItem) {

                _m_selected_column = static_cast<data_key>(view->iSubItem);
                _m_sort_order[_m_selected_column] = list_view_default_sort().at(_m_selected_column);
            } else {
                // Else swap the current selection
                _m_sort_order[_m_selected_column] =
                    (_m_sort_order[_m_selected_column] == ORDER_NORMAL) ? ORDER_REVERSE : ORDER_NORMAL;
            }

            for (const auto & [key, order] : _m_sort_order) {
                if (key == _m_selected_column) {
                    list->set_sort_arrow(key, (order == ORDER_NORMAL) ? list_view::UpArrow : list_view::DownArrow);
                } else {
                    list->set_sort_arrow(key, list_view::NoArrow);
                }
            }

            static auto sort_func = [](LPARAM lparam1, LPARAM lparam2, LPARAM info) {
                nao_view const* view = reinterpret_cast<nao_view const*>(info);

                return view->controller.order_items(
                    reinterpret_cast<item_data*>(lparam1), reinterpret_cast<item_data*>(lparam2),
                    view->selected_column(), view->selected_column_order());
            };

            list->sort(sort_func, this);
            break;
        }

        case NM_RCLICK: {
            NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(nm);

            if (item->iItem >= 0) {
                list_view* list = _m_main_window->left()->list();

                item_data* data = list->get_item_data<item_data*>(item->iItem);

                if (data) {
                    ClientToScreen(list->handle(), &item->ptAction);
                    controller.create_context_menu(data, item->ptAction);
                }
            }
            break;
        }

        default: break;
    }
}

void nao_view::create_preview(preview_element_type type) {
    preview_ptr preview;

    switch (type) {
        case PREVIEW_LIST_VIEW:
            preview = std::make_unique<list_view_preview>(this);
            break;
    }

    _m_main_window->right()->set_preview(std::move(preview));
}

void nao_view::clear_preview() const {
    _m_main_window->right()->remove_preview();
}

void nao_view::list_view_preview_fill(const std::vector<list_view_row>& items) const {
    list_view_preview* p = dynamic_cast<list_view_preview*>(_m_main_window->right()->get_preview());
    
    if (!p) {
        throw std::runtime_error("current preview is not a list_view_preview");
    }

    if (items.empty()) {
        return;
    }

    p->fill(items);
}

void nao_view::execute_context_menu(const context_menu& menu, POINT pt) {
    HMENU popup = CreatePopupMenu();

    MENUITEMINFOW item {
        .cbSize = sizeof(MENUITEMINFOW),
        .fMask = MIIM_DATA | MIIM_STRING | MIIM_ID
    };

    std::map<UINT, std::reference_wrapper<const std::function<void()>>> menu_functions;

    UINT id = 1;
    for (const auto& [text, function] : menu) {
        if (text.empty()) {
            if (menu.back().text != text) {
                // Empty indicates separator, ommit trailing separators
                static MENUITEMINFOW separator_item {
                    .cbSize = sizeof(MENUITEMINFOW),
                    .fMask = MIIM_TYPE,
                    .fType = MFT_SEPARATOR
                };

                InsertMenuItemW(popup, -1, true, &separator_item);
            }
        } else {
            auto wide = utils::utf16(text);
            item.dwTypeData = wide.data();
            item.dwItemData = reinterpret_cast<UINT_PTR>(&function);
            item.wID = id;

            menu_functions.emplace(id, std::ref(function));

            ++id;

            InsertMenuItemW(popup, -1, true, &item);
        }
    }

    UINT selected = TrackPopupMenuEx(popup,
        TPM_TOPALIGN | TPM_LEFTALIGN | TPM_VERPOSANIMATION | TPM_RETURNCMD | TPM_NONOTIFY,
        pt.x, pt.y, _m_main_window->left()->handle(), nullptr);

    if (selected > 0 && menu_functions.at(selected).get()) {
        auto func = new std::function<void()>(menu_functions.at(selected).get());
        controller.post_message(TM_EXECUTE_FUNC, 0, func);
    }

    DestroyMenu(popup);
}


main_window* nao_view::window() const {
    return _m_main_window.get();
}

data_key nao_view::selected_column() const {
    return _m_selected_column;
}

sort_order nao_view::selected_column_order() const {
    return _m_sort_order.at(_m_selected_column);
}




preview::preview(nao_view* view) : ui_element(view->window()->right()), view(view) {

}

list_view_preview::list_view_preview(nao_view* view) : preview(view)
    , _m_sort_order(nao_view::list_view_default_sort()), _m_selected_column(KEY_NAME) {
    std::wstring class_name = load_wstring(IDS_LIST_VIEW_PREVIEW);

    if (!_initialised) {
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

        _initialised = true;
    }

    auto [x, y, width, height] = parent()->dimensions();

    HWND handle = create_window(class_name, L"", WS_CHILD | WS_VISIBLE | SS_SUNKEN,
        { 0, 0, width, height }, parent(),
        new wnd_init(this, &list_view_preview::_wnd_proc));

    ASSERT(handle);
}

void list_view_preview::fill(std::vector<list_view_row> items) {
    std::sort(items.begin(), items.end(), [this](const list_view_row& first, const list_view_row& second) {
        return view->controller.order_items(
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
}

list_view* list_view_preview::get_list() const {
    return _m_list.get();
}

bool list_view_preview::wm_create(CREATESTRUCTW* create) {
    _m_list = std::make_unique<list_view>(this, nao_view::list_view_header(), nao_view::shell_image_list());
    _m_list->set_column_alignment(2, list_view::Right);

    return true;
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

                            return preview->view->controller.order_items(
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
                            view->controller.list_view_preview_clicked(CLICK_DOUBLE_ITEM,
                                _m_list->get_item_data(item->iItem));
                        }
                        break;
                    }

                    case NM_RCLICK: {
                        NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(nm);

                        if (item->iItem >= 0) {
                            item_data* data = _m_list->get_item_data<item_data*>(item->iItem);

                            if (data) {
                                ClientToScreen(_m_list->handle(), &item->ptAction);
                                view->controller.create_context_menu(data, item->ptAction);
                            }
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
