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

    // Preview has changed, fetch new preview
    TM_PREVIEW_CHANGED,

    TM_MODEL_LAST,

    TM_CONTROLLER_FIRST,

    // LPARAM: std::function<void()>* that should be deleted
    TM_EXECUTE_FUNC,

    TM_CONTROLLER_LAST,

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

enum click_event {
    CLICK_FIRST,

    //// Begin argumentless messages
    CLICK_MOVE_UP,
    //// End argumentless messages

    //// Begin messages with a void* argument
    CLICK_DOUBLE_ITEM,
    CLICK_SINGLE_ITEM,
    //// End messages with a void* argument


    CLICK_LAST
};

class nao_controller {
    public:
    explicit nao_controller();
    ~nao_controller() = default;

    // Message pump
    int pump();

    DWORD main_threadid() const;

    template <typename W = WPARAM, typename L = LPARAM>
    std::enable_if_t<
        std::is_convertible_v<W, WPARAM> &&
        std::is_convertible_v<L, LPARAM>>
        post_message(nao_thread_message message, W wparam = 0, L lparam = 0) const {
        PostThreadMessageW(_m_main_threadid, message, wparam, lparam);
    }

    template <typename W, typename L>
    std::enable_if_t<!std::is_convertible_v<L, LPARAM>>
        post_message(nao_thread_message message, W wparam = 0, L lparam = 0) const {

        post_message(message, wparam, reinterpret_cast<LPARAM>(lparam));
    }

    // A view element has been clicked
    void clicked(click_event which);
    void clicked(click_event which, void* arg);

    void list_view_preview_clicked(click_event which, void* arg);

    // Returns the relative ordering between 2 items,
    // returns -1 if the order is [first, second], returns 1
    // if the order is [second, first], or 0 if equivalent
    int order_items(item_data* first, item_data* second, data_key key, sort_order order);

    // Construct the items needed for a context menu of the given item
    void create_context_menu(item_data* data, POINT pt);

    // Move to an absolute path
    void move_to(const std::string& to);

    private:
    // Handle custom messages on the main thread
    void _handle_message(nao_thread_message msg, WPARAM wparam, LPARAM lparam);

    // Retrieve the current provider and fill the view from that
    void _refresh_view();

    // Retrieve the current preview provider and item and display it
    void _refresh_preview();

    // Transforms an item_data to a list_view_row
    static list_view_row _transform_data_to_row(const item_data& data);
    static std::vector<list_view_row> _transform_data_to_row(const std::vector<item_data>& data);

    protected:
    nao_view view;
    nao_model model;

    private:
    thread_pool _m_worker;

    const DWORD _m_main_threadid;
};

