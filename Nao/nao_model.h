#pragma once

#include "item_provider.h"

#include <deque>

class nao_view;
class nao_controller;

class nao_model {
    std::string _m_path;
    public:
    explicit nao_model(nao_view& view, nao_controller& controller);
    nao_model() = delete;

    void setup();

    void move_to(std::string path);
    void move_up();

    const std::string& current_path() const;

    const item_provider_ptr& current_provider() const;

    private:
    void _create_tree(const std::string& to);
    item_provider_ptr _provider_for(std::string path) const;

    protected:
    nao_view& view;
    nao_controller& controller;

    private:
    using istream_type = item_provider::istream_type;

    std::deque<item_provider_ptr> _m_tree;
};

/*
#include "frameworks.h"

#include "item_data.h"
#include "thread_pool.h"

#include <deque>

class main_window;
class right_window;
class list_view;
class line_edit;
class push_button;
class ui_element;
class audio_player;

class controller;

class data_model {
    public:
    data_model() = delete;
    explicit data_model(controller& controller, std::string initial_path);
    ~data_model() = default;

    enum sort_order : int8_t {
        SortOrderNone,
        SortOrderNormal,
        SortOrderReverse
    };

    // item_provider stream
    using stream = item_provider_factory::stream;

    
     * Messages,
     *
     * WPARAM:
     *   LOWORD: true if LPARAM should be freed, false if not
     *   HIWORD: true if condition variable should be notified, false if not
     
    enum messages : UINT {
        First = WM_USER,

        *
         * Simply execute a function on the main thread
         *
         * LPARAM pointer to a std::function<void()>;
         *
        ExecuteFunction,

        *
         * Set the preview element in _m_right
         *
         * LPARAM: pointer to a create_preview_async struct
         *
        CreatePreviewElement,

        *
         * Delete the current preview element
         *
         * No parameters
         *
        ClearPreviewElement,

        *
         * Insert an item into the specified list_view
         *
         * LPARAM: Pointer to an insert_element_async struct
         *
        InsertElementAsync,

        Last
    };

    enum preview_type : int {
        PreviewNone,
        PreviewListView,
        PreviewAudioPlayer
    };

    struct create_preview_async {
        std::function<std::shared_ptr<ui_element>()> creator;
        preview_type type;
    };

    struct insert_element_async {
        std::weak_ptr<list_view> list;
        std::vector<std::string> elements;
        int icon;
        item_data* data;
    };

    void set_window(const std::weak_ptr<ui_element>& window);
    void set_right(const std::weak_ptr<ui_element>& right);
    void set_list_view(const std::weak_ptr<ui_element>& list_view);
    void set_path_edit(const std::weak_ptr<ui_element>& path_edit);
    void set_up_button(const std::weak_ptr<ui_element>& up);

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
        std::weak_ptr<list_view> list;
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

        bool first_time { true };
        bool is_shown;
        item_provider* provider;
        item_data* data;

        sorted_list_view list;
        std::weak_ptr<audio_player> player;

        preview_type type;
    };

    item_provider* _get_provider(std::string path, bool return_on_error = false);
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
    void _selected_dir(item_data* data);
    void _selected_item(item_data* data);
    void _apply_preview(item_provider* p);
    void _sort(sorted_list_view& list, int col) const;

    void _preview_item_list(item_provider* p);
    void _preview_audio_player(item_provider* p);

    // Function used for sorting
    static int CALLBACK _sort_impl(LPARAM lparam1, LPARAM lparam2, LPARAM info);

    void _show_in_explorer(menu_state& state) const;

    // UI controller
    controller& _m_controller;

    std::string _m_path;

    std::deque<std::unique_ptr<item_provider>> _m_providers;
    
    std::weak_ptr<main_window> _m_window;
    std::weak_ptr<right_window> _m_right;
    std::weak_ptr<line_edit> _m_path_edit;
    std::weak_ptr<push_button> _m_up_button;

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

*/