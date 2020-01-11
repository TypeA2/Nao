#pragma once

#include <string>
#include <deque>
#include <stdexcept>

class item_provider;
class main_window;
class list_view;
class line_edit;

class data_model {
    public:
    data_model() = delete;
    explicit data_model(std::wstring initial_path);
    ~data_model();

    void set_window(main_window* window);
    void set_listview(list_view* listview);
    void set_path_edit(line_edit* path_edit);

    main_window* window() const;
    list_view* listview() const;
    line_edit* path_edit() const;

    void startup();

    private:
    item_provider* _get_provider();

    std::wstring _m_path;

    std::deque<item_provider*> _m_providers;
    
    main_window* _m_window;
    list_view* _m_listview;
    line_edit* _m_path_edit;
};

