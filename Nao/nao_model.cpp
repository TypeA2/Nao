#include "nao_model.h"

#include "utils.h"
#include "filesystem_utils.h"
#include "item_provider_factory.h"
#include "file_info.h"
#include "binary_stream.h"
#include "nao_controller.h"

#include <filesystem>

nao_model::nao_model(nao_view& view, nao_controller& controller) : view(view), controller(controller) {

}

void nao_model::setup() {
    move_to("C:\\Users\\Nuan\\Downloads");
}

void nao_model::move_to(std::string path) {
    std::string old_path = _m_path;
    
    // New path
    // Root should stay root, else take the absolute path
    if (path != "\\") {
        path = std::filesystem::absolute(path).string();
        if (path.back() != '\\') {
            path.push_back('\\');
        }
    }

    utils::coutln("move from", old_path, "to", path);

    _create_tree(path);

    _m_path = path;
    
    controller.post_message(TM_CONTENTS_CHANGED);
}

void nao_model::move_up() {
    if (_m_tree.size() > 2) {
        // More than 2 items, we're inside a drive
        move_to(_m_path + "..");
    } else if (_m_tree.size() == 2) {
        // Going up to drive view
        move_to("\\");
    } else {
        throw std::runtime_error("cannot move up from root");
    }
}

void nao_model::move_down(item_data* to) {
    const auto& current_elements = _m_tree.back()->data();

    auto find_func = [&to](const item_data& data) {
        return to == &data;
    };

    if (std::find_if(current_elements.begin(), current_elements.end(), find_func) == current_elements.end()) {
        // Not in current view, try preview

        if (_m_preview_provider) {
            const auto& preview_elements = _m_preview_provider->data();

            if (std::find_if(preview_elements.begin(), preview_elements.end(), find_func) == preview_elements.end()) {
                throw std::runtime_error("element not child of preview provider");
            }

            move_to(_m_preview_provider->get_path() + to->name);
        } else {
            throw std::runtime_error("element not child of current provider");
        }
    } else {
        // Element present, move down
        move_to(to->drive ? std::string { to->drive_letter, ':', '\\' } : (_m_path + to->name));
    }
}

void nao_model::fetch_preview(item_data* item) {
    const std::vector<item_data>& current_elements = _m_tree.back()->data();

    auto find_func = [&item](const item_data& data) {
        return item == &data;
    };

    if (std::find_if(current_elements.begin(), current_elements.end(), find_func) == current_elements.end()) {
        throw std::runtime_error("element not child of current provider");
    }

    std::string path = item->drive ? std::string { item->drive_letter, ':', '\\' } : (_m_path + item->name + '\\');

    item_provider_ptr p = _provider_for(path);

    if (p) {
        _m_preview_provider = std::move(p);
        controller.post_message(TM_PREVIEW_CHANGED);
    } else {
        utils::coutln("not found");
    }
}

void nao_model::clear_preview() {
    _m_preview_provider.reset();
}

const std::string& nao_model::current_path() const {
    return _m_path;
}

const item_provider_ptr& nao_model::current_provider() const {
    return _m_tree.back();
}

const item_provider_ptr& nao_model::preview_provider() const {
    return _m_preview_provider;
}

void nao_model::_create_tree(const std::string& to) {
    // Modify the current tree to match the supplied path

    // Build tree from highest level common parent
    std::string current_path;

    // Remove elements from the current queue until we reach a child of the target path
    while (_m_tree.size() > 1) { // But never remove the last node

        // Get
        item_provider_ptr p = _m_tree.back();

        // If this provider represents any child of the target path or the path itself
        if (fs_utils::is_child(p->get_path(), to) || p->get_path() == to) {
            // It's the one we need
            current_path = p->get_path();
            break;
        }

        _m_tree.pop_back();
    }

    // If we're moving to the root
    if (_m_tree.size() == 1 && to == "\\") {
        return;
    }

    // Must have at least 1 element
    if (_m_tree.empty()) {
        _m_tree.push_back(_provider_for("\\"));
    }

    while (current_path != to) {
        // Construct the path for the next element from the target path
        current_path = to.substr(0, to.find_first_of('\\', current_path.size() + 1) + 1);

        auto p = _provider_for(current_path);
        
        if (!p) {
            throw std::runtime_error("unsupported element in tree at " + current_path);
        }

        _m_tree.push_back(p);
    }

    // Tree should be done
}

item_provider_ptr nao_model::_provider_for(std::string path) {
    // If a preview is set, it should just be the next element
    if (_m_preview_provider && path == _m_preview_provider->get_path()) {
        auto p = _m_preview_provider;

        clear_preview();

        return p;
    }
    // Requesting root
    if (path == "\\" && !_m_tree.empty()) {
        return _m_tree.front();
    }

    // If the element was already created and present at the back
    if (!_m_tree.empty() && _m_tree.back()
        && _m_tree.back()->get_path() == path) {
        return _m_tree.back();
    }

    file_info info(path);

    // If the path was not found
    if (!info) {
        // It may be a file that was hidden by the trailing separator
        if (path.back() == '\\') {
            path.pop_back();
            info = file_info(path);
        }
        
        // If not, it may be virtual
    }

    if (info.invalid()) {
        // Virtual (in-archive) file
    } else {
        if (info.directory()) {
            // file_info considers "\" a directory as well, so that is included in this
            return item_provider_factory::create(nullptr, path);
        }

        // Create file stream
        istream_type stream = std::make_shared<istream_type::element_type>(path);

        if (stream->good()) {
            return item_provider_factory::create(stream, path);
        }
    }

    return nullptr;
}

/*
void data_model::menu_clicked(short id) {
    item_data* data = _m_menu.data;
    switch (id) {
        case CtxOpen: {
            if (data->dir || !data->drive) {
                move(_m_menu.path + data->name);
            } else if (data->drive) {
                move({ data->drive_letter, ':', '\\' });
            }
            break;
        }

        case CtxShowInExplorer: {
            _show_in_explorer(_m_menu);
            break;
        }

        default: return;
    }


void data_model::_clear_preview() {
    ASSERT(std::this_thread::get_id() != _m_main_thread);

    delete _m_preview.provider;
    _m_preview.provider = nullptr;

    switch (_m_preview.type) {
        case PreviewListView:
            _m_preview.list.list.reset();
            break;
        case PreviewAudioPlayer:
            _m_preview.player.reset();
            break;

        default: break;
    }

    _m_preview.type = PreviewNone;

    _m_preview.is_shown = false;

    PostMessageW(handle(), ClearPreviewElement, MAKEWPARAM(false, true), 0);
    std::unique_lock lock(_m_message_mutex);
    _m_cond.wait(lock, [this] { return !_m_right.lock()->preview().lock().get(); });
}




    // Fit columns
    for (int i = 0; i < list_view->column_count() - 1; ++i) {
        int min = 0;

        if (i == 0) {
            min = list_view->width() / list_view->column_count();
        }

        list_view->set_column_width(i, LVSCW_AUTOSIZE, min);
    }

    // Fill remainder with last column
    list_view->set_column_width(list_view->column_count() - 1, LVSCW_AUTOSIZE_USEHEADER);

    // Items already sorted, we're done
}

void data_model::_context_menu(sorted_list_view& list, POINT pt, bool preview) {
    auto list_view = list.list.lock();
    int index = list_view->item_at(pt);
    ASSERT(index < list_view->item_count());

    ClientToScreen(list_view->handle(), &pt);

    _m_menu.is_preview = preview;

    item_data* data = list_view->get_item_data<item_data>(index);
    _m_menu.data = data;
    _m_menu.index = index;
    _m_menu.path = preview ? _m_preview->get_name() : _m_path;

    HMENU popup = CreatePopupMenu();

    // Should the "Show in explorer" entry be appended
    bool insert = false;
    // Could have clicked outside items
    if (data) {
        if (!data->drive) {
            if (GetFileAttributesW(utils::utf16(_m_menu.path + data->name).c_str()) != INVALID_FILE_ATTRIBUTES) {
                insert = true;
            }
        } else {
            if ((GetLogicalDrives() >> (data->drive - 'A')) & 1) {
                insert = true;
            }
        }

        bool open = false;
        if (data->dir || data->drive) {
            open = true;
        } else if (item_provider* p = _get_provider(_m_menu.path + data->name, true); p && p->count() > 0) {
            delete p;
            open = true;
        }

        if (open) {
            InsertMenuW(popup, -1, MF_BYPOSITION | MF_STRING, CtxOpen, L"Open");

            // Separator if there's another item that follows
            if (insert) {
                InsertMenuW(popup, -1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
            }
        }
    }

    if (insert || index < 0) {
        InsertMenuW(popup, -1, MF_BYPOSITION | MF_STRING, CtxShowInExplorer, L"Show in explorer");
    }

    TrackPopupMenuEx(popup, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_VERPOSANIMATION,
        pt.x, pt.y, list_view->parent()->handle(), nullptr);

    DestroyMenu(popup);
}

void data_model::_sort(sorted_list_view& list, int col) const {
    std::vector default_order = listview_default_sort();

    if (list.selected != col) {
        // Set default order
        list.order[col] = default_order[col];
    } else {
        list.order[list.selected]
            = (list.order[col] == SortOrderReverse) ? SortOrderNormal : SortOrderReverse;
    }

    list.selected = col;

    auto list_view = list.list.lock();
    for (int i = 0; i < int(std::size(list.order)); ++i) {
        if (i == col) {
            list_view->set_sort_arrow(i,
                (list.order[i] == SortOrderReverse) ? list_view::DownArrow : list_view::UpArrow);
        } else {
            list_view->set_sort_arrow(i, list_view::NoArrow);
        }
    }

    ListView_SortItems(list_view->handle(), &data_model::_sort_impl, MAKELPARAM(col, list.order[col]));
}

void data_model::_preview_item_list(item_provider* p) {
    if (_m_preview.first_time) {
        _m_preview.list.selected = _m_list_view.selected;
        _m_preview.list.order = _m_list_view.order;
    }

    create_preview_async preview {
        .creator = [this] {
            auto ptr = std::make_shared<list_view>(_m_right);
            ptr->init();
            ptr->set_columns(listview_header());
            ptr->set_image_list(shell_image_list());
            return ptr;
        },
        .type    = PreviewListView
    };

    PostMessageW(handle(), CreatePreviewElement, MAKEWPARAM(false, true), LPARAM(&preview));

    auto right = _m_right.lock();
    std::unique_lock lock(_m_message_mutex);
    _m_cond.wait(lock, [right] { return !!right->preview().lock(); });

    _m_preview.type = PreviewListView;
    _m_preview.is_shown = true;
    _m_preview.list.list = std::dynamic_pointer_cast<list_view>(right->preview().lock());
    _m_preview.provider = p;
    _fill(_m_preview.list, p);

    _m_preview.list.list.lock()->set_sort_arrow(_m_preview.list.selected,
        (_m_preview.list.order[_m_preview.list.selected] == SortOrderReverse)
        ? list_view::DownArrow : list_view::UpArrow);

    // Sort by name first
    if (_m_preview.first_time) {
        _m_preview.first_time = false;

        PostMessageW(handle(), ExecuteFunction,
            MAKEWPARAM(true, false),
            LPARAM(new std::function<void()>([this] {
                _m_preview.list.order[0] = SortOrderReverse;
                _sort(_m_preview.list, 0);
                })));
    }
}

void data_model::_preview_audio_player(item_provider* p) {
    create_preview_async preview {
        .creator = [this, p] { return p->preview_element(_m_right.lock()); },
        .type    = PreviewAudioPlayer
    };

    PostMessageW(handle(), CreatePreviewElement, MAKEWPARAM(false, true), LPARAM(&preview));

    std::unique_lock lock(_m_message_mutex);
    _m_cond.wait(lock, [this] { return !!_m_right.lock()->preview().lock().get(); });

    _m_preview.type = PreviewAudioPlayer;
    _m_preview.is_shown = true;
    _m_preview.provider = p;

    _m_preview.player = std::dynamic_pointer_cast<audio_player>(_m_right.lock()->preview().lock());

}




void data_model::_show_in_explorer(menu_state& state) const {
    (void) this;

    const sorted_list_view& list = state.is_preview ? _m_preview.list : _m_list_view;

    LPITEMIDLIST idl;
    if (state.index >= 0) {
        item_data* data = list.list.lock()->get_item_data<item_data>(state.index);
        idl = ILCreateFromPathW(utils::utf16(_m_menu.path + data->name).c_str());
    } else {
        idl = ILCreateFromPathW(utils::utf16(_m_menu.path).c_str());
    }

    if (idl) {
        SHOpenFolderAndSelectItems(idl, 0, nullptr, 0);

        ILFree(idl);
    }
}

*/
