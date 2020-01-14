#pragma once

#include "frameworks.h"

#include "item_provider.h"

#include <deque>
#include <stdexcept>
#include <vector>
#include <atomic>

class main_window;
class left_window;
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

    data_model() = delete;
    explicit data_model(std::wstring initial_path);
    ~data_model();

    void set_window(main_window* window);
    void set_left_window(left_window* left);
    void set_listview(list_view* listview);
    void set_path_edit(line_edit* path_edit);
    void set_up_button(push_button* up);

    main_window* window() const;
    left_window* left() const;
    list_view* listview() const;
    line_edit* path_edit() const;
    push_button* up_button() const;

    HWND handle() const;

    const std::wstring& path() const;

    static std::vector<std::string> listview_header();
    static std::vector<sort_order> listview_default_sort();

    void startup();

    void sort_list(int col);

    void move_relative(const std::wstring& rel);
    void move(const std::wstring& path);

    void clicked(int index);
    void context_menu(POINT pt);
    void menu_clicked(short id);

    void show_in_explorer(int index) const;

    private:
    // Context menu actions
    enum context_menu_action : uint16_t {
        CtxFirst = 16384,
        CtxOpen,
        CtxShowInExplorer
    };

    item_provider* _get_provider(const std::wstring& path);

    // Fill view with items from the specified path
    void _fill();

    // Thread locking, stop if false
    bool _lock();

    // Build provider queue for the current path
    void _build();

    // Function used for sorting
    static int CALLBACK _sort_impl(LPARAM lparam1, LPARAM lparam2, LPARAM info);

    std::wstring _m_path;

    // Only allow 1 async operation at a time
    std::atomic<bool> _m_lock;
    std::deque<item_provider*> _m_providers;
    
    main_window* _m_window;
    left_window* _m_left;
    list_view* _m_listview;
    line_edit* _m_path_edit;
    push_button* _m_up_button;

    // Sorting
    int _m_selected_col;
    std::vector<sort_order> _m_sort_order;

    // Menus
    item_data* _m_menu_item;
    int _m_menu_item_index;
};

