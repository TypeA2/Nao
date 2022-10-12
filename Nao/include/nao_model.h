#pragma once

#include <libnao/util/logging.h>

#include "path.h"

class nao_presenter;

class nao_model {
    NAO_LOGGER(nao_model);
    nao_presenter& _presenter;

    /* Current raw path */
    path _path;

    public:

    explicit nao_model(nao_presenter& presenter);

    void set_path(std::string_view target);
    [[nodiscard]] const path& path() const;

    void up();
};
