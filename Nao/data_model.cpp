#include "data_model.h"

#include "frameworks.h"
#include "utils.h"
#include "item_provider.h"
#include "item_provider_factory.h"
#include "main_window.h"
#include "list_view.h"
#include "line_edit.h"

data_model::data_model(std::wstring initial_path)
    : _m_path { std::move(initial_path) }
    , _m_window { }
    , _m_listview { }
    , _m_path_edit { } {
    
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
}

void data_model::set_path_edit(line_edit* path_edit) {
    ASSERT(!_m_path_edit && path_edit);
    _m_path_edit = path_edit;
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



void data_model::startup() {
    ASSERT(_m_window && _m_listview && _m_path_edit);

    _m_path_edit->set_text(_m_path);
}



item_provider* data_model::_get_provider() {
    item_provider* p = nullptr;
    if (GetFileAttributesW(_m_path.data()) & FILE_ATTRIBUTE_DIRECTORY) {
        std::stringstream ss;
        ss << utils::utf8(_m_path);

        p = item_provider_factory::create(ss, _m_window);
    }

    if (!p) {
        MessageBoxW(_m_window->handle(), (L"Could not open " + _m_path).c_str(), L"Error", MB_OK | MB_ICONEXCLAMATION);
        return nullptr;
    }

    _m_providers.push_back(p);
    return p;
}
