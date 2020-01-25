#include "data_model.h"

#include "utils.h"
#include "item_provider.h"
#include "item_provider_factory.h"
#include "main_window.h"
#include "line_edit.h"
#include "list_view.h"
#include "push_button.h"
#include "right_window.h"
#include "com_thread.h"
#include "audio_player.h"

#include <ShlObj_core.h>

#include <filesystem>
#include <thread>
#include <future>
#include <fstream>

data_model::data_model(std::string initial_path)
    : _m_path { std::move(initial_path) }
    , _m_window { }
    , _m_right { }
    , _m_path_edit { }
    , _m_up_button { }
    , _m_list_view { }
    , _m_preview { }
    , _m_worker(1)
    , _m_main_thread { std::this_thread::get_id() } {
    
}

data_model::~data_model() {
    while (!_m_providers.empty()) {
        item_provider* p = _m_providers.back();
        delete p;
        _m_providers.pop_back();
    }
}


void data_model::set_window(main_window* window) {
    ASSERT(!_m_window && window);
    _m_window = window;
}

void data_model::set_right(right_window* right) {
    ASSERT(!_m_right && right);
    _m_right = right;
}

void data_model::set_list_view(list_view* list_view) {
    ASSERT(!_m_list_view && list_view);

    _m_list_view = list_view;

    _m_list_view.order.resize(list_view->column_count());
}

void data_model::set_path_edit(line_edit* path_edit) {
    ASSERT(!_m_path_edit && path_edit);
    _m_path_edit = path_edit;
}

void data_model::set_up_button(push_button* up) {
    ASSERT(!_m_up_button && up);
    _m_up_button = up;
}

main_window* data_model::get_window() const {
    ASSERT(_m_window);
    return _m_window;
}

right_window* data_model::get_right() const {
    ASSERT(_m_right);
    return _m_right;
}

list_view* data_model::get_list_view() const {
    ASSERT(_m_list_view);
    return _m_list_view;
}

line_edit* data_model::get_path_edit() const {
    ASSERT(_m_path_edit);
    return _m_path_edit;
}

push_button* data_model::get_up_button() const {
    return _m_up_button;
}

HWND data_model::handle() const {
    ASSERT(_m_window);
    return _m_window->handle();
}

const std::string& data_model::path() const {
    return _m_path;
}



std::vector<std::string> data_model::listview_header() {
    return { "Name", "Type", "Size", "Compressed" };
}

std::vector<data_model::sort_order> data_model::listview_default_sort() {
    return { SortOrderNormal, SortOrderNormal, SortOrderReverse, SortOrderReverse };
}

IImageList* data_model::shell_image_list() {
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



void data_model::startup() {
    ASSERT(_m_window && _m_right && _m_list_view && _m_path_edit && _m_up_button);

    // "Size" alignment
    _m_list_view->set_column_alignment(2, list_view::Right);

    if (_m_path.empty()) {
        _m_path = "\\";
    }

    _m_providers.push_back(_get_provider("\\"));
    move(_m_path);

    // Default sort
    _m_list_view.order[0] = SortOrderReverse;
    sort_list(0);
}

void data_model::sort_list(int col) {
    _sort(_m_list_view, col);
}

void data_model::sort_preview(int col) {
    if (_m_right->type() == PreviewListView) {
        _sort(_m_preview.list, col);
    }
}


void data_model::move_relative(const std::string& rel) {
    _m_worker.push_detached(com_thread::bind([this, rel] {
        if (rel == "..") {
            delete _m_providers.back();
            _m_providers.pop_back();

            // Current directory is a drive (C:\)
            if (_m_path.size() == 3 &&
                _m_path[0] >= 'A' &&
                _m_path[0] <= 'Z' &&
                _m_path.substr(1, 2) == ":\\") {
                _move("\\");
                return;
            }
        }

        _move(std::filesystem::absolute(_m_path + '\\' + rel).string());
        }));
}

void data_model::move(const std::string& path) {
    _m_worker.push_detached(com_thread::bind(std::bind(&data_model::_move, this, path)));
}

void data_model::opened(int index) {
    _m_worker.push_detached(
        com_thread::bind_cond(
            [index] { return index >= 0; },
            std::bind(&data_model::_opened, this, std::ref(_m_list_view), index)));
}

void data_model::opened_preview(int index) {
    _m_worker.push_detached(
        com_thread::bind_cond(
            [index] { return index >= 0; },
            std::bind(&data_model::_opened, this, std::ref(_m_preview.list), index)));
}

void data_model::context_menu(POINT pt) {
    _context_menu(_m_list_view, pt, false);
}

void data_model::context_menu_preview(POINT pt) {
    _context_menu(_m_preview.list, pt, true);
}

void data_model::selected(POINT pt) {
    _m_worker.push_detached(
        com_thread::bind_cond(
            [this, pt] { return _m_list_view->item_at(pt) >= 0; },
            std::bind(&data_model::_selected, this, pt)));
}

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
    
    _m_menu = { };
}



void data_model::handle_message(messages msg, WPARAM wparam, LPARAM lparam) {
    bool _delete = LOWORD(wparam);
    bool _notify = HIWORD(wparam);

    switch (msg) {
        case ExecuteFunction: {
            auto func = reinterpret_cast<std::function<void()>*>(lparam);

            (*func)();

            if (_delete) {
                delete func;
            }
            break;
        }

        case CreatePreviewElement: {
            auto cpa = reinterpret_cast<create_preview_async*>(lparam);

            _m_right->set_preview(cpa->creator(), cpa->type);

            if (_delete) {
                delete cpa;
            }
            break;
        }

        case ClearPreviewElement: {
            _m_right->clear_preview();
            break;
        }

        case InsertElementAsync: {
            auto item = reinterpret_cast<insert_element_async*>(lparam);

            item->list->add_item(
                item->elements, item->icon, LPARAM(item->data));

            if (_delete) {
                delete item;
            }
            break;
        }

        default: break;
    }

    if (_notify) {
        std::unique_lock lock(_m_message_mutex);
        _m_cond.notify_all();
    }
}



data_model::sorted_list_view::operator list_view*() const {
    return list;
}

data_model::sorted_list_view& data_model::sorted_list_view::operator=(list_view* list) {
    this->list = list;
    return *this;
}

list_view* data_model::sorted_list_view::operator->() const noexcept {
    return list;
}

item_provider* data_model::preview_state::operator->() const noexcept {
    return provider;
}



item_provider* data_model::_get_provider(std::string path, bool return_on_error) {
    // When only moving up 1 level 
    if (!_m_providers.empty()) {
        std::string name = _m_providers.back()->get_name();

        if (path.size() > 1 && name.back() == '\\' && path.back() != '\\') {
            name.pop_back();
        }

        if (name == path) {
            return _m_providers.back();
        }
    }

    if (_m_preview.is_shown) {
        const std::string& name = _m_preview->get_name();
        
        if (name == path || name == path.substr(0, path.size() - 1)) {
            return _m_preview.provider;
        }
    }

    item_provider* p = nullptr;

    DWORD attribs = GetFileAttributesW(utils::utf16(path).c_str());

    if (attribs == INVALID_FILE_ATTRIBUTES) {
        if (path.back() == '\\') {
            path.pop_back();
        }

        attribs = GetFileAttributesW(utils::utf16(path).c_str());
    }

    

    if (attribs != INVALID_FILE_ATTRIBUTES) {
        if (attribs & FILE_ATTRIBUTE_DIRECTORY) {
            // "\" is also treated as a directory
            p = item_provider_factory::create(nullptr, path, *this);

        } else {
            stream s = std::make_unique<binary_stream>(path);

            if (s->good()) {
                p = item_provider_factory::create(s, path, *this);
            }
        }
    }

    if (!return_on_error && !p) {
        MessageBoxW(handle(), utils::utf16("Could not open " + path).c_str(),
            L"Error", MB_OK | MB_ICONEXCLAMATION);
        return nullptr;
    }

    return p;
}

void data_model::_clear_preview() {
    ASSERT(std::this_thread::get_id() != _m_main_thread);

    delete _m_preview.provider;
    _m_preview.provider = nullptr;

    switch (_m_preview.type) {
        case PreviewListView:
            _m_preview.list = nullptr;
            break;
        case PreviewAudioPlayer:
            _m_preview.player = nullptr;
            break;

        default: break;
    }

    _m_preview.type = PreviewNone;

    _m_preview.is_shown = false;

    PostMessageW(handle(), ClearPreviewElement, MAKEWPARAM(false, true), 0);
    std::unique_lock lock(_m_message_mutex);
    _m_cond.wait(lock, [this] { return !_m_right->preview(); });
}



void data_model::_fill(sorted_list_view& list, item_provider* provider) {
    list->clear([](void* data) { delete reinterpret_cast<item_data*>(data); });
    
    item_provider* p = provider ? provider : _get_provider(_m_path);
    if (!p) {
        return;
    }
    // Sort columns
    std::vector<item_data*> items(p->count());
    for (size_t i = 0; i < p->count(); ++i) {
        items[i] = new item_data;
        *items[i] = p->data(i);
    }

    std::sort(items.begin(), items.end(), [list](item_data* left, item_data* right) -> bool {
        return _sort_impl(LPARAM(left), LPARAM(right),
            MAKELPARAM(list.selected, list.order[list.selected])) == -1;
        });

    // TODO maybe go on main thread from here on?

    // Add items
    for (item_data* data : items) {
        PostMessageW(handle(), InsertElementAsync, MAKEWPARAM(true, false),
            LPARAM(new insert_element_async {
            list,
            { data->name, data->type,
                (data->size == 0)
                    ? std::string()
                    : data->size_str,
                (data->compression == 0.)
                    ? std::string()
                    : (std::to_string(int64_t(data->compression / 100.)) + '%') },
            data->icon,
            data
                }));
    }

    // Fit columns
    for (int i = 0; i < list->column_count() - 1; ++i) {
        int min = 0;

        if (i == 0) {
            min = list->width() / list->column_count();
        }

        list->set_column_width(i, LVSCW_AUTOSIZE, min);
    }

    // Fill remainder with last column
    list->set_column_width(list->column_count() - 1, LVSCW_AUTOSIZE_USEHEADER);

    // Items already sorted, we're done
}

void data_model::_build() {
    // Need root element
    if (_m_path.size() > 1 && _m_path.back() != '\\') {
        _m_path.push_back('\\');
    }

    std::string current;
    // Never remove root
    while (_m_providers.size() > 1) {
        item_provider* p = _m_providers.back();
        std::string name = p->get_name();

        // Stop if this provider is part of the current path
        if (name.size() <= _m_path.size() && // Equal or smaller (this should be a substring)
            name == _m_path.substr(0, name.size())) {
            current = name;
            break;
        }

        utils::coutln("removing", name);

        delete p;
        _m_providers.pop_back();
    }

    // Root-only base case
    if (_m_providers.size() == 1 && _m_path == "\\") {
        return;
    }

    utils::coutln("current, path", current, _m_path);
    // Build path
    while (current != _m_path) {
        current = _m_path.substr(0,
            _m_path.find_first_of('\\', current.size() + 1) + 1);

        _m_providers.push_back(_get_provider(current));
    }
}

void data_model::_opened(sorted_list_view& list, int index) {
    ASSERT(index < list->item_count());

    item_data* data = list->get_item_data<item_data>(index);

    if (data->dir) {
        if (list.list == _m_preview.list.list) {
            // Preview list is selected
            _move(std::filesystem::absolute(
                _m_path + _m_preview.data->name + '\\' + data->name).string());
        } else {
            // Normal list is clicked
            _move(std::filesystem::absolute(_m_path + data->name).string());
        }
    } else if (data->drive) {
        _move({ data->drive_letter, ':', '\\' });
    } else if (_m_preview.is_shown) {
        if (_m_preview.type == PreviewListView) {
            _move(_m_path + data->name + '\\');
        }
    }
}

void data_model::_move(const std::string& path) {
    std::string old_path = _m_path;
    _m_path = path;
    if (_m_path.back() != '\\') {
        _m_path.push_back('\\');
    }

    utils::coutln("from", old_path, "to", _m_path);

    _m_path_edit->set_text(utils::utf16(_m_path));

    if (
        // Went deeper
        _m_path.size() > old_path.size()

        // Same tree
        && (old_path == "\\" || _m_path.substr(0, old_path.size()) == old_path)

        // Preview currently shown
        && _m_preview.is_shown) {

        item_provider* p = _m_preview.provider;
        _m_preview.provider = nullptr;

        _clear_preview();

        _m_providers.push_back(p);
    } else {
        // Went up
        if (old_path.size() > _m_path.size()
            && old_path.substr(0, _m_path.size()) == _m_path) {
            _clear_preview();
        }

        _build();
    }

    _fill(_m_list_view);

    _m_up_button->set_enabled(_m_path != "\\");
}

void data_model::_context_menu(sorted_list_view& list, POINT pt, bool preview) {
    int index = list->item_at(pt);
    ASSERT(index < list->item_count());

    ClientToScreen(list->handle(), &pt);

    _m_menu.is_preview = preview;

    item_data* data = list->get_item_data<item_data>(index);
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
        pt.x, pt.y, list->parent()->handle(), nullptr);

    DestroyMenu(popup);
}

void data_model::_selected(POINT pt) {
    int index = _m_list_view->item_at(pt);

    ASSERT(index < _m_list_view->item_count());

    item_data* data = _m_list_view->get_item_data<item_data>(index);

    // If the selected item changed
    if (_m_preview.data != data) {
        _clear_preview();

        _m_preview.data = data;

        if (data->dir || data->drive) {
            _selected_dir(data);
        } else {
            _selected_item(data);
        }
    }
}

void data_model::_selected_dir(item_data* data) {
    std::string path = _m_path + data->name + '\\';

    if (data->drive) {
        path = { data->drive_letter, ':', '\\' };
    }

    // Get preview provider
    if (item_provider* p = _get_provider(path, true); p) {
        _apply_preview(p);
    }
}

void data_model::_selected_item(item_data* data) {
    std::string path = _m_path + data->name;

    if (item_provider* p = _get_provider(path, true); p) {
        _apply_preview(p);
    }
}

void data_model::_apply_preview(item_provider* p) {
    switch (p->preview()) {
        case PreviewListView:
            _preview_item_list(p);
            break;

        case PreviewAudioPlayer:
            _preview_audio_player(p);
            break;

        default:
            _m_preview.type = PreviewNone;
            delete p;
    }
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

    for (int i = 0; i < int(std::size(list.order)); ++i) {
        if (i == col) {
            list->set_sort_arrow(i,
                (list.order[i] == SortOrderReverse) ? list_view::DownArrow : list_view::UpArrow);
        } else {
            list->set_sort_arrow(i, list_view::NoArrow);
        }
    }

    ListView_SortItems(list->handle(), &data_model::_sort_impl, MAKELPARAM(col, list.order[col]));
}

int data_model::_sort_impl(LPARAM lparam1, LPARAM lparam2, LPARAM info) {
    item_data* item1 = reinterpret_cast<item_data*>(lparam1);
    item_data* item2 = reinterpret_cast<item_data*>(lparam2);

    if (!item1 || !item2) {
        return 0;
    }

    sort_order order = static_cast<sort_order>(HIWORD(info) & 0xFF);

    int first1 = 0;
    int first2 = 0;

    switch (order) {
        case SortOrderNormal:
            first1 = -1;
            first2 = 1;
            break;

        case SortOrderReverse:
            first1 = 1;
            first2 = -1;
            break;

        default:
            break;
    }

    // Directories on top
    if (!item1->dir != !item2->dir) {
        return item1->dir ? -1 : 1;
    }

    const auto& f = std::use_facet<std::ctype<std::string::value_type>>(std::locale());

    auto cmp = [&](const std::string& left, const std::string& right) -> int {
        return std::lexicographical_compare(
            left.begin(), left.end(), right.begin(), right.end(),
            [&f](std::string::value_type a, std::string::value_type b) {
                return f.tolower(a) < f.tolower(b);
            }) ? first1 : first2;
    };

    switch (LOWORD(info)) {
        case 0: // Name, alphabetically
            if (item1->name == item2->name) { return 0; }

            return cmp(item1->name, item2->name);
        case 1: { // Type, alphabetically
            if (item1->type == item2->type) {
                // Fallback on name
                return cmp(item1->name, item2->name);
            }

            return cmp(item1->type, item2->type); }

        case 2: // File size
            if (item1->size == item2->size) {
                // Fallback on name
                return cmp(item1->name, item2->name);
            }

            return (item1->size < item2->size) ? first1 : first2;

        case 3: // Compression ratio
            if (item1->compression == item2->compression) {
                // Fallback on name
                return cmp(item1->name, item2->name);
            }

            return (item1->compression < item2->compression) ? first1 : first2;

        default: return 0;
    }
}



void data_model::_preview_item_list(item_provider* p) {
    if (_m_preview.first_time) {
        _m_preview.list.selected = _m_list_view.selected;
        _m_preview.list.order = _m_list_view.order;
    }

    create_preview_async preview {
        [this] {
            return new list_view(_m_right, listview_header(), shell_image_list());
        },
        PreviewListView
    };

    PostMessageW(handle(), CreatePreviewElement, MAKEWPARAM(false, true), LPARAM(&preview));

    std::unique_lock lock(_m_message_mutex);
    _m_cond.wait(lock, [this] { return !!_m_right->preview(); });

    _m_preview.type = PreviewListView;
    _m_preview.is_shown = true;
    _m_preview.list = dynamic_cast<list_view*>(_m_right->preview());
    _m_preview.provider = p;
    _fill(_m_preview.list, p);

    _m_preview.list->set_sort_arrow(_m_preview.list.selected,
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
        .creator = [this, p] { return p->preview_element(_m_right); },
        .type    = PreviewAudioPlayer
    };

    PostMessageW(handle(), CreatePreviewElement, MAKEWPARAM(false, true), LPARAM(&preview));

    std::unique_lock lock(_m_message_mutex);
    _m_cond.wait(lock, [this] { return !!_m_right->preview(); });

    _m_preview.type = PreviewAudioPlayer;
    _m_preview.is_shown = true;
    _m_preview.provider = p;

    audio_player* player = dynamic_cast<audio_player*>(_m_right->preview());
    _m_preview.player = player;

    
}




void data_model::_show_in_explorer(menu_state& state) const {
    (void) this;

    const sorted_list_view& list = state.is_preview ? _m_preview.list : _m_list_view;

    LPITEMIDLIST idl;
    if (state.index >= 0) {
        item_data* data = list->get_item_data<item_data>(state.index);
        idl = ILCreateFromPathW(utils::utf16(_m_menu.path + data->name).c_str());
    } else {
        idl = ILCreateFromPathW(utils::utf16(_m_menu.path).c_str());
    }

    if (idl) {
        SHOpenFolderAndSelectItems(idl, 0, nullptr, 0);

        ILFree(idl);
    }
}
