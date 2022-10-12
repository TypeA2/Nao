#include "nao_model.h"

#include "nao_presenter.h"

#include <filesystem>

nao_model::nao_model(nao_presenter& presenter) : _presenter{ presenter } {
    
}

void nao_model::set_path(std::string_view target) {
    _path.set(target);
    _presenter.path_changed();
}

const path& nao_model::path() const {
    return _path;
}

void nao_model::up() {
    _path.up();
    _presenter.path_changed();
}
