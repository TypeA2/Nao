#pragma once

#include "nao_model.h"
#include "nao_view.h"

#include "thread_pool.h"

// Thread messages to handle processing on the main thread
enum nao_thread_message : unsigned {
    TM_FIRST = WM_APP + 1,

    TM_INSERT_ELEMENT,

    TM_LAST
};

enum thread_message_wparam : WPARAM {
    DoNothing     = 0b00,
    Free          = 0b01,
    Notify        = 0b10,
    FreeAndNotify = Free | Notify
};

class nao_controller {
    public:
    explicit nao_controller();
    ~nao_controller() = default;

    // Message pump
    int pump();

    DWORD main_threadid() const;

    private:
    // Handle custom messages on the main thread
    void _handle_message(nao_thread_message msg, WPARAM wparam, LPARAM lparam);

    protected:
    nao_view view;
    nao_model model;

    private:
    thread_pool _m_worker;

    const DWORD _m_main_threadid;
};

