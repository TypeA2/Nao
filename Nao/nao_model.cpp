#include "nao_model.h"

#include "utils.h"
#include "filesystem_utils.h"
#include "item_provider_factory.h"
#include "file_info.h"
#include "binary_stream.h"
#include "nao_controller.h"
#include "steam_utils.h"

#include <filesystem>

nao_model::nao_model(nao_view& view, nao_controller& controller) : view(view), controller(controller) {

}

void nao_model::setup() {
    move_to(steam_utils::game_path("NieRAutomata", "data"));
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
    move_to(to->path());
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

    _m_preview_provider = std::move(p);
    controller.post_message(TM_PREVIEW_CHANGED);
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

const item_provider_ptr& nao_model::parent_provider() const {
    if (_m_tree.size() < 2) {
        static item_provider_ptr null = nullptr;
        return null;
    }

    return _m_tree[_m_tree.size() - 2];
}

bool nao_model::can_open(item_data* data) {
    if (data->drive) {
        if (item_provider_ptr p = _provider_for({ data->drive_letter, ':', '\\' }); p) {
            return true;
        }
    } else {
        if (item_provider_ptr p = _provider_for(data->provider->get_path() + data->name); p) {
            return true;
        }
    }

    return false;
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
