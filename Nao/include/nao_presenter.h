#pragma once

#include "nao_model.h"
#include "nao_window.h"


class nao_presenter {
    nao_model _model;
    nao_window _window;

    public:
    explicit nao_presenter();

    [[nodiscard]] int run();
};
