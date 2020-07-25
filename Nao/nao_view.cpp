#include "nao_view.h"

#include "nao_controller.h"

#include "main_window.h"
#include "left_window.h"
#include "right_window.h"

#include "line_edit.h"
#include "push_button.h"
#include "list_view.h"
#include "preview.h"

#include "utils.h"

#include <nao/logging.h>
#include <nao/strings.h>

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
            nao::coutln("failed to retrieve main image list");
            return nullptr;
        }

        return imglist;
    }

    // Increment ref count if already initialised
    imglist->AddRef();

    return imglist;
}

nao_view::nao_view(nao_controller& controller) : controller(controller) {

}

nao_view::~nao_view() {
    (void) this;
}

void nao_view::setup() {
    _main_window = std::make_unique<main_window>(*this);
}

void nao_view::set_path(const std::string& path) const {
    auto& left = _main_window->left();

    // Display the current path in the line edit above
    left.path().set_text(path);

    // Only enable the up button whenever we are not at the root path
    left.view_up().set_enabled(path != "\\");
}

void nao_view::clear_view(const std::function<void(void*)>& deleter) const {
    _main_window->left().list().clear(deleter);
}

void nao_view::fill_view(std::vector<list_view_row> items) const {
    if (items.empty()) {
        return;
    }

    list_view& list = _main_window->left().list();

    std::sort(items.begin(), items.end(), [this](const list_view_row& first, const list_view_row& second) {
        return controller.order_items(
            static_cast<item_data*>(const_cast<void*>(first.data)),
            static_cast<item_data*>(const_cast<void*>(second.data)),
            _selected_column, selected_column_order()) == -1;
        });

    for (const auto& [key, order] : _sort_order) {
        if (key == _selected_column) {
            list.set_sort_arrow(key, (order == ORDER_NORMAL) ? list_view::UpArrow : list_view::DownArrow);
        } else {
            list.set_sort_arrow(key, list_view::NoArrow);
        }
    }

    for (const auto& [name, type, size, compressed,
            icon ,data] : items) {
        list.add_item({ name, type, size, compressed }, icon, data);
    }

    // Fit columns
    for (int i = 0; i < list.column_count() - 1; ++i) {
        int64_t min = 0;

        if (i == 0) {
            min = list.width() / list.column_count();
        }

        list.set_column_width(i, LVSCW_AUTOSIZE, min);
    }
}

void nao_view::button_clicked(view_button_type which) const {

    switch (which) {
        case BUTTON_BROWSE: {
            com_ptr<IFileOpenDialog> dialog;
            HASSERT(dialog.CreateInstance(CLSID_FileOpenDialog));

            FILEOPENDIALOGOPTIONS options;
            HRESULT hr = dialog->GetOptions(&options);
            if (SUCCEEDED(hr)) {
                dialog->SetOptions(options | FOS_PICKFOLDERS);

                hr = dialog->Show(_main_window->handle());
                if (SUCCEEDED(hr)) {
                    com_ptr<IShellItem> item;
                    hr = dialog->GetResult(&item);
                    if (SUCCEEDED(hr)) {
                        LPWSTR path;
                        hr = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
                        if (FAILED(hr)) {
                            nao::coutln("Failed to get path");
                        } else {
                            controller.move_to(nao::to_utf8(path));
                        }
                    }
                }
            }
            return;
        }

        default: break;
    }

    static std::map<view_button_type, click_event> map {
        { BUTTON_UP, CLICK_MOVE_UP },
    };

    controller.clicked(map[which]);
}

void nao_view::list_clicked(NMHDR* nm) {
    switch (nm->code) {
        case NM_DBLCLK: {
            // Double-click, opened an item
            NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(nm);

            if (item->iItem >= 0) {
                controller.clicked(CLICK_DOUBLE_ITEM,
                    _main_window->left().list().get_item_data(item->iItem));
            }
            break;
        }

        case NM_CLICK: {
            // Single left-click, preview item (if possible)
            NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(nm);
            if (item->iItem >= 0) {
                controller.clicked(CLICK_SINGLE_ITEM,
                    _main_window->left().list().get_item_data(item->iItem));
            }
            break;
        }

        case LVN_COLUMNCLICK: {
            // Clicked on a column, sort based on this column
            NMLISTVIEW* view = reinterpret_cast<NMLISTVIEW*>(nm);
            list_view& list = _main_window->left().list();

            if (view->iItem != -1) {
                break;
            }

            // If a different column was selected, use the default sort
            if (_selected_column != view->iSubItem) {

                _selected_column = static_cast<data_key>(view->iSubItem);
                _sort_order[_selected_column] = list_view_default_sort().at(_selected_column);
            } else {
                // Else swap the current selection
                _sort_order[_selected_column] =
                    (_sort_order[_selected_column] == ORDER_NORMAL) ? ORDER_REVERSE : ORDER_NORMAL;
            }

            for (const auto & [key, order] : _sort_order) {
                if (key == _selected_column) {
                    list.set_sort_arrow(key, (order == ORDER_NORMAL) ? list_view::UpArrow : list_view::DownArrow);
                } else {
                    list.set_sort_arrow(key, list_view::NoArrow);
                }
            }

            static auto sort_func = [](LPARAM lparam1, LPARAM lparam2, LPARAM info) {
                nao_view const* view = reinterpret_cast<nao_view const*>(info);

                return view->controller.order_items(
                    reinterpret_cast<item_data*>(lparam1), reinterpret_cast<item_data*>(lparam2),
                    view->selected_column(), view->selected_column_order());
            };

            list.sort(sort_func, this);
            break;
        }

        case NM_RCLICK: {
            NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(nm);

            list_view& list = _main_window->left().list();
            ClientToScreen(list.handle(), &item->ptAction);
            if (item->iItem >= 0) {
                item_data* data = list.get_item_data<item_data*>(item->iItem);
                if (data) {
                    controller.create_context_menu(data, item->ptAction);
                }
            } else {
                // Clicked outside of item, pass null
                controller.create_context_menu(nullptr, item->ptAction);
            }
            break;
        }

        default: break;
    }
}

void nao_view::set_preview(preview_ptr preview) const {
    _main_window->right().set_preview(std::move(preview));
}

void nao_view::clear_preview() const {
    _main_window->right().remove_preview();
}

void nao_view::execute_context_menu(const context_menu& menu, POINT pt) const {
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
            auto wide = nao::to_utf16(text);
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
        pt.x, pt.y, _main_window->left().handle(), nullptr);

    if (selected > 0 && menu_functions.at(selected).get()) {
        auto func = new std::function<void()>(menu_functions.at(selected).get());
        controller.post_message(TM_EXECUTE_FUNC, 0, func);
    }

    DestroyMenu(popup);
}

void nao_view::select(void* data) const {
    list_view& list = _main_window->left().list();
    if (int i = list.index_of(data); i >= 0) {
        list.select(i);
    } else if (list_view_preview* p = dynamic_cast<list_view_preview*>(_main_window->right().get_preview()); p) {
        controller.clicked(CLICK_DOUBLE_ITEM, list.selected_data());
        controller.clicked(CLICK_SINGLE_ITEM, data);
        controller.post_work(std::bind(&nao_view::select, this, p->list().selected_data()));
        return;
    }
    
    controller.clicked(CLICK_SINGLE_ITEM, data);
}

main_window* nao_view::window() const {
    return _main_window.get();
}

data_key nao_view::selected_column() const {
    return _selected_column;
}

sort_order nao_view::selected_column_order() const {
    return _sort_order.at(_selected_column);
}

nao_controller& nao_view::get_controller() const {
    return controller;
}
