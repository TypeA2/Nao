#pragma once

#include "frameworks.h"

#include <string>
#include <deque>
#include <stdexcept>
#include <vector>
#include <atomic>

class item_provider;
class main_window;
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


    data_model() = delete;
    explicit data_model(std::wstring initial_path);
    ~data_model();

    void set_window(main_window* window);
    void set_listview(list_view* listview);
    void set_path_edit(line_edit* path_edit);
    void set_up_button(push_button* up);

    main_window* window() const;
    list_view* listview() const;
    line_edit* path_edit() const;
    push_button* up_button() const;

    static std::vector<std::string> listview_header();
    static std::vector<sort_order> listview_default_sort();

    void startup();

    void sort_list(int col);

    void move_relative(const std::wstring& rel);


    private:
    // LPARAM for list items
    struct list_item_data {
        std::wstring name;
        std::wstring type;
        int64_t size {};
        std::wstring size_str;
        double compression {};
        int icon {};
        bool dir {};
    };

    item_provider* _get_provider(const std::wstring& path);

    // Fill view with items from the specified path
    void _fill();

    // Thread locking, stop if false
    bool _lock();

    // Function used for sorting
    static int CALLBACK _sort_impl(LPARAM lparam1, LPARAM lparam2, LPARAM info);

    std::wstring _m_path;

    // Only allow 1 async operation at a time
    std::atomic<bool> _m_lock;
    std::deque<item_provider*> _m_providers;
    
    main_window* _m_window;
    list_view* _m_listview;
    line_edit* _m_path_edit;
    push_button* _m_up_button;

    // Sorting
    int _m_selected_col;
    std::vector<sort_order> _m_sort_order;
};

