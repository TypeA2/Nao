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
    static constexpr char path_prefix[] = R"(\\?\)";

    explicit nao_model(nao_presenter& presenter);

    void set_path(std::string_view target);

    private:
    static std::string _make_path(std::string_view source);
};
