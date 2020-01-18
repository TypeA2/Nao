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
    void set_listview(list_view* listview);
    void set_path_edit(line_edit* path_edit);
    void set_up_button(push_button* up);

    main_window* window() const;
    right_window* right() const;
    list_view* listview() const;
    line_edit* path_edit() const;
    push_button* up_button() const;

    HWND handle() const;

    const std::wstring& path() const;

    static std::vector<std::string> listview_header();
    static std::vector<sort_order> listview_default_sort();

    static IImageList* shell_image_list();

    void startup();

    void sort_list(int col);

    void move_relative(const std::wstring& rel);
    void move(const std::wstring& path);

    void clicked(int index);
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

    item_provider* _get_provider(const std::wstring& path, bool return_on_error = false);

    // Fill view with items from the specified path
    void _fill();

    // Thread locking, stop if false
    bool _lock();
    void _unlock();

    // Build provider queue for the current path
    void _build();

    // Use the provider as a preview
    void _preview(item_provider* p);

    // Function used for sorting
    static int CALLBACK _sort_impl(LPARAM lparam1, LPARAM lparam2, LPARAM info);

    std::wstring _m_path;

    // Only allow 1 async operation at a time
    std::atomic<bool> _m_locked;
    std::deque<item_provider*> _m_providers;
    
    main_window* _m_window;
    right_window* _m_right;
    list_view* _m_listview;
    line_edit* _m_path_edit;
    push_button* _m_up_button;

    // Sorting
    int _m_selected_col;
    std::vector<sort_order> _m_sort_order;

    int _m_preview_selected;
    std::vector<sort_order> _m_preview_order;

    // Menus
    item_data* _m_menu_item;
    int _m_menu_item_index;

    // Preview
    item_data* _m_preview_data;
    item_provider* _m_preview_provider;

    // Event handler worker (pool)
    thread_pool _m_worker;
    std::mutex _m_mutex;
    std::condition_variable _m_cond;
};

