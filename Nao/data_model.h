#pragma once

#include <string>
#include <deque>
#include <stdexcept>
#include <vector>

class item_provider;
class main_window;
class list_view;
class line_edit;

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

    main_window* window() const;
    list_view* listview() const;
    line_edit* path_edit() const;

    static std::vector<std::string> listview_header();
    static std::vector<sort_order> listview_default_sort();

    void startup();

    void sort_list(int col);


    private:
    // LPARAM for list items
    struct list_item_data {
        std::wstring name;
        std::wstring type;
        int64_t size {};
        double compression {};
        int icon {};
        bool dir {};
    };

    item_provider* _get_provider(const std::wstring& path);

    std::wstring _m_path;

    std::deque<item_provider*> _m_providers;
    
    main_window* _m_window;
    list_view* _m_listview;
    line_edit* _m_path_edit;

    // Sorting
    int _m_selected_col;
    std::vector<sort_order> _m_sort_order;
};

