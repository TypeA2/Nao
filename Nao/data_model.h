#pragma once

#include "frameworks.h"

#include "item_provider.h"
#include "thread_pool.h"

#include <deque>
#include <stdexcept>

class main_window;
class right_window;
class list_view;
class line_edit;
class push_button;

class data_model {
    public:
    enum sort_order : int8_t {
        None,
        Normal,
        Reverse
    };

    // LPARAM for list items
    using item_data = item_provider::item_data;

    // Messages
    enum messages : UINT {
        First = WM_USER,

        /*
         * Set the preview element in _m_right
         *
         * WPARAM: 0 if LPARAM should be `delete`ed, nonzero if not
         * LPARAM: pointer to a std::function<ui_element*()>
         */
        CreatePreviewElement,

        /*
         * Delete the current preview element
         *
         * No parameters
         */
        ClearPreviewElement,

        Last
    };

    data_model() = delete;
    explicit data_model(std::wstring initial_path);
    ~data_model();

    void set_window(main_window* window);
    void set_right(right_window* right);
    void set_list_view(list_view* list_view);
    void set_path_edit(line_edit* path_edit);
    void set_up_button(push_button* up);

    main_window* get_window() const;
    right_window* get_right() const;
    list_view* get_list_view() const;
    line_edit* get_path_edit() const;
    push_button* get_up_button() const;

    HWND handle() const;

    const std::wstring& path() const;

    static std::vector<std::string> listview_header();
    static std::vector<sort_order> listview_default_sort();

    static IImageList* shell_image_list();

    void startup();

    void sort_list(int col);

    void move_relative(const std::wstring& rel);
    void move(const std::wstring& path);

    void opened(int index);
    void context_menu(POINT pt);
    void selected(POINT pt);
    void menu_clicked(short id);

    void show_in_explorer(int index) const;

    void handle_message(messages msg, WPARAM wparam, LPARAM lparam);

    private:
    // Context menu actions
    enum context_menu_action : uint16_t {
        CtxFirst = 16384,
        CtxOpen,
        CtxShowInExplorer
    };

    // Encapsulate a list_view and it's sorting information
    struct sorted_list_view {
        operator list_view* () const;
        sorted_list_view& operator=(list_view* list);
        list_view* operator->() const noexcept;

        list_view* list;
        int selected;
        std::vector<sort_order> order;
    };

    item_provider* _get_provider(const std::wstring& path, bool return_on_error = false);
    void _clear_preview();

    // Fill view with items from the specified path
    void _fill(sorted_list_view& list,
        item_provider* provider = nullptr);

    // Thread locking, stop if false
    bool _lock();
    void _unlock();

    // Build provider queue for the current path
    void _build();

    void _opened(int index);
    void _move(const std::wstring& path);
    void _selected(POINT pt);
    void _sort(sorted_list_view& list, int col) const;

    // Function used for sorting
    static int CALLBACK _sort_impl(LPARAM lparam1, LPARAM lparam2, LPARAM info);

    std::wstring _m_path;

    // Only allow 1 async operation at a time
    std::atomic<bool> _m_locked;
    std::deque<item_provider*> _m_providers;
    
    main_window* _m_window;
    right_window* _m_right;
    line_edit* _m_path_edit;
    push_button* _m_up_button;

    // Sorting
    sorted_list_view _m_list_view;
    sorted_list_view _m_preview_list;

    // Menus
    item_data* _m_menu_item;
    int _m_menu_item_index;

    // Preview
    item_data* _m_preview_data;
    item_provider* _m_preview_provider;

    // Event handler worker (pool)
    thread_pool _m_worker;
    std::mutex _m_message_mutex;
    std::condition_variable _m_cond;

    const std::thread::id _m_main_thread;
};

