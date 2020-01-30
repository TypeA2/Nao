#pragma once

#include "nao_model.h"
#include "nao_view.h"

#include "thread_pool.h"

// Thread messages to handle processing on the main thread
enum nao_thread_message : unsigned {
    TM_FIRST = WM_APP + 1,



    TM_MODEL_FIRST,

    // Model view contents changed.
    // No parameters
    TM_CONTENTS_CHANGED,

    TM_MODEL_LAST,



    TM_VIEW_FIRST,

    // A button has been clicked
    // WPARAM: view_butten_type of the button
    TM_BUTTON_CLICKED,


    TM_VIEW_LAST,



    TM_LAST
};

enum thread_message_wparam_action : WPARAM {
    DoNothing     = 0b00,
    Free          = 0b01,
    Notify        = 0b10,
    FreeAndNotify = Free | Notify
};

struct thread_message {
    std::condition_variable& condition;
};

class nao_controller {
    public:
    explicit nao_controller();
    ~nao_controller() = default;

    // Message pump
    int pump();

    DWORD main_threadid() const;

    template <typename W = WPARAM, typename L = LPARAM>
    void post_message(nao_thread_message message, W wparam = 0, L lparam = 0) const {
        PostThreadMessageW(_m_main_threadid, message, wparam, lparam);
    }

    private:
    // Handle custom messages on the main thread
    void _handle_message(nao_thread_message msg, WPARAM wparam, LPARAM lparam);

    // Retrieve the current provider and fill the view from that
    void _refresh_view() const;

    // A view button has been clicked
    void _handle_button_clicked(view_button_type which);

    protected:
    nao_view view;
    nao_model model;

    private:
    thread_pool _m_worker;

    const DWORD _m_main_threadid;
};

