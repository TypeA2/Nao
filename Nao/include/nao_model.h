#pragma once

#include <libnao/util/logging.h>

#include <string>

class nao_presenter;

class nao_model {
    NAO_LOGGER(nao_model);
    nao_presenter& _presenter;

    /* Current raw path */
    std::string _path;

    public:
    static constexpr char path_prefix[] = R"(\\?\)";

    explicit nao_model(nao_presenter& presenter);

    void set_path(std::string_view target);

    private:
    static std::string _make_path(std::string_view source);
};
