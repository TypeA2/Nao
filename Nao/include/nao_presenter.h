#pragma once

#include "nao_model.h"
#include "nao_window.h"

#include <libnao/util/thread_pool.h>

class nao_presenter {
    NAO_LOGGER(nao_presenter);

    nao_model _model;
    nao_window _window;

    /* Single for executing actions sequentially */
    nao::thread_pool _worker{ 1 };

    public:
    explicit nao_presenter();

    [[nodiscard]] int run();

    void up();
    void refresh();
};
