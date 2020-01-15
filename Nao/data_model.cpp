#include "data_model.h"

#include "utils.h"
#include "item_provider.h"
#include "item_provider_factory.h"
#include "main_window.h"
#include "line_edit.h"
#include "list_view.h"
#include "push_button.h"

#include <ShlObj_core.h>

#include <filesystem>
#include <thread>

data_model::data_model(std::wstring initial_path)
    : _m_path { std::move(initial_path) }
    , _m_lock { true } 
    , _m_window { }
    , _m_listview { }
    , _m_path_edit { }
    , _m_up_button { }
    , _m_selected_col { }
    , _m_menu_item { }
    , _m_menu_item_index { -1 } {
    
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

void data_model::set_listview(list_view* listview) {
    ASSERT(!_m_listview && listview);

    _m_listview = listview;

    _m_sort_order.resize(_m_listview->column_count());
}

void data_model::set_path_edit(line_edit* path_edit) {
    ASSERT(!_m_path_edit && path_edit);
    _m_path_edit = path_edit;
}

void data_model::set_up_button(push_button* up) {
    ASSERT(!_m_up_button && up);
    _m_up_button = up;
}

main_window* data_model::window() const {
    ASSERT(_m_window);
    return _m_window;
}

list_view* data_model::listview() const {
    ASSERT(_m_listview);
    return _m_listview;
}

line_edit* data_model::path_edit() const {
    ASSERT(_m_path_edit);
    return _m_path_edit;
}

push_button* data_model::up_button() const {
    return _m_up_button;
}

HWND data_model::handle() const {
    ASSERT(_m_window);
    return _m_window->handle();
}

const std::wstring& data_model::path() const {
    return _m_path;
}

std::vector<std::string> data_model::listview_header() {
    return { "Name", "Type", "Size", "Compressed" };
}

std::vector<data_model::sort_order> data_model::listview_default_sort() {
    return { Normal, Normal, Reverse, Reverse };
}



void data_model::startup() {
    ASSERT(_m_window && _m_listview && _m_path_edit && _m_up_button);

    // "Size" alignment
    _m_listview->set_column_alignment(2, list_view::Right);

    if (_m_path.empty()) {
        _m_path = L"\\";
    }

    _m_providers.push_back(_get_provider(L"\\"));
    move(_m_path);

    // Default sort
    _m_sort_order[0] = Reverse;
    sort_list(0);
}

void data_model::sort_list(int col) {
    _m_selected_col = col;

    std::vector default_order = listview_default_sort();

    if (_m_selected_col != col) {
        // Set default order
        _m_sort_order[col] = default_order[col];
    } else {
        _m_sort_order[_m_selected_col]
            = (_m_sort_order[_m_selected_col] == Reverse) ? Normal : Reverse;
    }

    _m_selected_col = col;

    for (int i = 0; i < int(std::size(_m_sort_order)); ++i) {
        if (i == col) {
            _m_listview->set_sort_arrow(i,
                (_m_sort_order[i] == Reverse) ? list_view::DownArrow : list_view::UpArrow);
        } else {
            _m_listview->set_sort_arrow(i, list_view::NoArrow);
        }
    }

    ListView_SortItems(_m_listview->handle(), &data_model::_sort_impl, MAKELPARAM(col, _m_sort_order[col]));
}

void data_model::move_relative(const std::wstring& rel) {
    if (rel == L"..") {
        delete _m_providers.back();
        _m_providers.pop_back();

        // A directory (C:\)
        if (_m_path.size() == 3 &&
            _m_path[0] >= L'A' &&
            _m_path[0] <= L'Z' &&
            _m_path.substr(1, 2) == L":\\") {
            move(L"\\");
            return;
        }
    }

    move(std::filesystem::absolute(_m_path + L'\\' + rel));
}

void data_model::move(const std::wstring& path) {
    if (!_lock()) {
        return;
    }

    std::wstring old_path = _m_path;

    _m_path = path;

    utils::coutln("from", old_path, "to", _m_path);

    // Go to root or not
    _m_up_button->set_enabled(_m_path != L"\\");

    _build();

    _fill();
}


void data_model::clicked(int index) {
    ASSERT(index < _m_listview->item_count());

    item_data* data = _m_listview->get_item_data<item_data>(index);

    if (data->dir) {
        move_relative(data->name);
    } else if (data->drive) {
        move({ data->drive_letter, L':', L'\\' });
    }
}

void data_model::context_menu(POINT pt) {
    (void) this;
    int index = _m_listview->item_at(pt);
    ASSERT(index < _m_listview->item_count());

    ClientToScreen(_m_listview->handle(), &pt);

    item_data* data = _m_listview->get_item_data<item_data>(index);
    _m_menu_item = data;
    _m_menu_item_index = index;

    HMENU popup = CreatePopupMenu();

    // Should the "Show in explorer" entry be appended
    bool insert = false;
    if (!data->drive) {
        if (GetFileAttributesW((_m_path + data->name).c_str()) != INVALID_FILE_ATTRIBUTES) {
            insert = true;
        }
    } else {
        if ((GetLogicalDrives() >> (data->drive - L'A')) & 1) {
            insert = true;
        }
    }

    if (data->dir || data->drive) {
        InsertMenuW(popup, -1, MF_BYPOSITION | MF_STRING, CtxOpen, L"Open");

        // Separator if there's another item that follows
        if (insert) {
            InsertMenuW(popup, -1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
        }
    }

    if (insert) {
        InsertMenuW(popup, -1, MF_BYPOSITION | MF_STRING, CtxShowInExplorer, L"Show in explorer");
    }

    
    TrackPopupMenuEx(popup, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_VERPOSANIMATION,
        pt.x, pt.y, _m_listview->parent()->handle(), nullptr);

    DestroyMenu(popup);
}

void data_model::menu_clicked(short id) {
    item_data* data = _m_menu_item;
    switch (id) {
        case CtxOpen: {
            if (data->dir || data->drive) {
                clicked(_m_menu_item_index);
            }
            break;
        }

        case CtxShowInExplorer: {
            show_in_explorer(_m_menu_item_index);
            break;
        }

        default: return;
    }

    _m_menu_item = nullptr;
    _m_menu_item_index = -1;
}

void data_model::show_in_explorer(int index) const {
    item_data* data = _m_listview->get_item_data<item_data>(index);
    LPITEMIDLIST idl = ILCreateFromPathW((_m_path + data->name).c_str());
    if (idl) {
        SHOpenFolderAndSelectItems(idl, 0, nullptr, 0);

        ILFree(idl);
    }
}



item_provider* data_model::_get_provider(const std::wstring& path) {
    if (!_m_providers.empty()) {
        std::wstring name = utils::utf16(_m_providers.back()->get_name());

        if (path.size() > 1 && name.back() == L'\\' && path.back() != L'\\') {
            name.pop_back();
        }

        if (name == path) {
            return _m_providers.back();
        }
    }

    item_provider* p = nullptr;

    if (GetFileAttributesW(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
        // "\" is also regarded a directory
        std::stringstream null;
        p = item_provider_factory::create(null, utils::utf8(path), *this);
    }

    if (!p) {
        MessageBoxW(handle(), (L"Could not open " + path).c_str(),
            L"Error", MB_OK | MB_ICONEXCLAMATION);
        return nullptr;
    }

    return p;
}

void data_model::_fill() {
    _m_path_edit->set_text(_m_path);

    _m_listview->clear([](void* data) { delete reinterpret_cast<item_data*>(data); });
    
    void(__cdecl * fwd)(void*) = [](void* args) {
        (void) CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        data_model* _this = reinterpret_cast<data_model*>(args);
        item_provider* p = _this->_get_provider(_this->_m_path);
        // Sort columns
        std::vector<item_data*> items(p->count());
        for (size_t i = 0; i < p->count(); ++i) {
            items[i] = new item_data;
            *items[i] = p->data(i);
        }

        std::sort(items.begin(), items.end(), [_this](item_data* left, item_data* right) -> bool {
            return _sort_impl(LPARAM(left), LPARAM(right),
                MAKELPARAM(_this->_m_selected_col, _this->_m_sort_order[_this->_m_selected_col])) == -1;
            });

        // Add items
        for (item_data* data : items) {
            _this->_m_listview->add_item(
                { data->name, data->type,
                    (data->size == 0)
                        ? std::wstring()
                        : data->size_str,
                    (data->compression == 0.)
                        ? std::wstring()
                        : (std::to_wstring(int64_t(data->compression / 100.)) + L'%') },
                data->icon, LPARAM(data));
        }

        // Fit columns
        for (int i = 0; i < _this->_m_listview->column_count() - 1; ++i) {
            int min = 0;

            if (i == 0) {
                min = _this->_m_listview->width() / _this->_m_listview->column_count();
            }

            _this->_m_listview->set_column_width(i, LVSCW_AUTOSIZE, min);
        }

        // Fill remainder with last column
        _this->_m_listview->set_column_width(_this->_m_listview->column_count() - 1, LVSCW_AUTOSIZE_USEHEADER);

        // Items already sorted, we're done

        _this->_m_lock = true;

        CoUninitialize();
    };

    size_t low;
    size_t high;
    GetCurrentThreadStackLimits(&low, &high);

    _beginthread(fwd, unsigned(high - low), this);
}

bool data_model::_lock() {
    if (!_m_lock) {
        return false;
    }

    _m_lock = true;
    return true;
}

void data_model::_build() {
    if (_m_path.size() > 1 && _m_path.back() != L'\\') {
        _m_path.push_back(L'\\');
    }

    std::wstring current;
    // Never remove root
    while (_m_providers.size() > 1) {
        item_provider* p = _m_providers.back();
        std::wstring name = utils::utf16(p->get_name());

        // Stop if this provider is part of the current path
        if (name.size() <= _m_path.size() && // Equal or smaller (this should be a substring)
            name == _m_path.substr(0, name.size())) {
            current = name;
            break;
        }

        delete p;
        _m_providers.pop_back();
    }

    // Root-only base case
    if (_m_providers.size() == 1 && _m_path == L"\\") {
        return;
    }

    // Build path
    while (current != _m_path) {
        current = _m_path.substr(0,
            _m_path.find_first_of(L'\\', current.size() + 1) + 1);

        _m_providers.push_back(_get_provider(current));
    }
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
        case Normal:
            first1 = -1;
            first2 = 1;
            break;

        case Reverse:
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

    const auto& f = std::use_facet<std::ctype<std::wstring::value_type>>(std::locale());

    auto cmp = [&](const std::wstring& left, const std::wstring& right) -> int {
        return std::lexicographical_compare(
            left.begin(), left.end(), right.begin(), right.end(),
            [&f](std::wstring::value_type a, std::wstring::value_type b) {
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
