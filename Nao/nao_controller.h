#pragma once

#include "nao_model.h"
#include "nao_view.h"

class nao_controller {
    public:
    explicit nao_controller();
    ~nao_controller() = default;

    protected:
    nao_view view;
    nao_model model;
};

