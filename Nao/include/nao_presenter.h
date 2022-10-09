#pragma once

#include "nao_model.h"
#include "nao_window.h"

#include <libnao/util/thread_pool.h>
#include <libnao/ui/event.h>

#include <magic_enum.hpp>

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

    friend struct magic_enum::customize::enum_range<thread_message>;

    DWORD _main_thread_id;

    public:
    explicit nao_presenter();

    [[nodiscard]] int run();

    [[nodiscard]] bool event_filter(nao::ui::event& e);

    void up();
    void refresh();
    void browse();
    void path_changed(std::string_view new_path);

    /*** Async functions, these are never called from the GUI thread  ***/

    void path_changed() const;

    private:
    void _new_path(std::string path);
};

template <>
struct magic_enum::customize::enum_range<nao_presenter::thread_message> {
    static constexpr int min = nao_presenter::thread_message::TM_FIRST;
    static constexpr int max = nao_presenter::thread_message::TM_LAST;
};
