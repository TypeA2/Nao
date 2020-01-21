#pragma once

#include "frameworks.h"

#include "item_provider.h"
#include "thread_pool.h"

#include <deque>

class main_window;
class right_window;
class list_view;
class line_edit;
class push_button;
class ui_element;

class data_model {
    public:
    enum sort_order : int8_t {
        SortOrderNone,
        SortOrderNormal,
        SortOrderReverse
    };

    // LPARAM for list items
    using item_data = item_provider::item_data;

    /*
     * Messages,
     *
     * WPARAM:
     *   LOWORD: true if LPARAM should be freed, false if not
     *   HIWORD: true if condition variable should be notified, false if not
     */
    enum messages : UINT {
        First = WM_USER,

        /*
         * Simply execute a function on the main thread
         *
         * LPARAM pointer to a std::function<void()>;
         */
        ExecuteFunction,

        /*
         * Set the preview element in _m_right
         *
         * LPARAM: pointer to a create_preview_async struct
         */
        CreatePreviewElement,

        /*
         * Delete the current preview element
         *
         * No parameters
         */
        ClearPreviewElement,

        /*
         * Insert an item into the specified list_view
         *
         * LPARAM: Pointer to an insert_element_async struct
         */
        InsertElementAsync,

        Last
    };

    enum preview_type {
        PreviewNone,
        PreviewListView
    };

    struct create_preview_async {
        std::function<ui_element*()> creator;
        preview_type type;
    };

    struct insert_element_async {
        list_view* list;
        std::vector<std::string> elements;
        int icon;
        item_data* data;
    };

    data_model() = delete;
    explicit data_model(std::string initial_path);
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

    const std::string& path() const;

    static std::vector<std::string> listview_header();
    static std::vector<sort_order> listview_default_sort();

    static IImageList* shell_image_list();

    void startup();

    void sort_list(int col);
    void sort_preview(int col);

    void move_relative(const std::string& rel);
    void move(const std::string& path);

    void opened(int index);
    void opened_preview(int index);
    void context_menu(POINT pt);
    void context_menu_preview(POINT pt);
    void selected(POINT pt);
    void menu_clicked(short id);

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

    // Data needed for an open menu
    struct menu_state {
        bool is_preview {};
        item_data* data {};
        int index {};
        std::string path;
    };

    // Data needed for a preview
    struct preview_state {
        item_provider* operator->() const noexcept;

        bool is_shown;
        item_provider* provider;
        item_data* data;
        sorted_list_view list;
    };

    item_provider* _get_provider(const std::string& path, bool return_on_error = false);
    void _clear_preview();

    // Fill view with items from the specified path
    void _fill(sorted_list_view& list,
        item_provider* provider = nullptr);

    // Build provider queue for the current path
    void _build();

    void _opened(sorted_list_view& list, int index);
    void _move(const std::string& path);
    void _context_menu(sorted_list_view& list, POINT pt, bool preview);
    void _selected(POINT pt);
    void _sort(sorted_list_view& list, int col) const;

    // Function used for sorting
    static int CALLBACK _sort_impl(LPARAM lparam1, LPARAM lparam2, LPARAM info);

    void _show_in_explorer(menu_state& state) const;

    std::string _m_path;

    std::deque<item_provider*> _m_providers;
    
    main_window* _m_window;
    right_window* _m_right;
    line_edit* _m_path_edit;
    push_button* _m_up_button;

    // Main list
    sorted_list_view _m_list_view;

    // Menus
    menu_state _m_menu;

    // Preview
    preview_state _m_preview;

    // Event handler worker (pool)
    thread_pool _m_worker;
    std::mutex _m_message_mutex;
    std::condition_variable _m_cond;

    const std::thread::id _m_main_thread;
};

