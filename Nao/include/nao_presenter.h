#pragma once

#include "nao_model.h"
#include "nao_window.h"

#include <libnao/util/thread_pool.h>
#include <libnao/ui/event.h>

class nao_presenter {
    NAO_LOGGER(nao_presenter);

    nao_model _model;
    nao_window _window;

    /* Single for executing actions sequentially */
    nao::thread_pool _worker{ 1 };

    enum thread_message : uint32_t {
        TM_FIRST = WM_APP + 1,

        /* Model's current path has changed */
        TM_PATH_CHANGED,

        TM_LAST,
    };

    public:
    explicit nao_presenter();

    [[nodiscard]] int run();

    [[nodiscard]] bool event_filter(nao::ui::event& e);

    void up();
    void refresh();

    /*** Async functions, these are never called from the GUI thread  ***/

    void path_changed() const;
};
