#include "nao_model.h"

#include "nao_presenter.h"

#include <filesystem>

nao_model::nao_model(nao_presenter& presenter) : _presenter{ presenter } {
    
}

void nao_model::set_path(std::string_view target) {
    _path.set(target);
    //_path = _make_path(target);
    _presenter.path_changed();
}

std::string nao_model::_make_path(std::string_view source) {
    // Actual path
    auto path = std::filesystem::path{ source }.make_preferred();

    // Explicit case for root, as we want this to show all drives on Win32
    if (equivalent(path, "/")) {
        return path_prefix;
    }

    // Else just prepend the prefix
    return path_prefix + path.string();
}
