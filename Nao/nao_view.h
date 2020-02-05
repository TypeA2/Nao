#pragma once

#include "frameworks.h"

#include "ui_element.h"
#include "item_provider.h"

#include <vector>
#include <string>
#include <map>

class nao_controller;
class main_window;

struct list_view_row {
    std::string name;
    std::string type;
    std::string size;
    std::string compressed;

    int icon { };
    void* data { };
};

enum view_button_type {
    BUTTON_NONE,

    BUTTON_UP,
    BUTTON_BROWSE
};

enum sort_order {
    ORDER_NONE,
    ORDER_NORMAL,
    ORDER_REVERSE
};

enum data_key {
    KEY_NAME = 0,
    KEY_TYPE,
    KEY_SIZE,
    KEY_COMP
};

struct context_menu_entry {
    std::string text;
    std::function<void()> func;
};

using context_menu = std::vector<context_menu_entry>;

class nao_view {
    public:
    static const std::vector<std::string>& list_view_header();
    static const std::map<data_key, sort_order>& list_view_default_sort();
    static IImageList* shell_image_list();

    explicit nao_view(nao_controller& controller);
    nao_view() = delete;
    ~nao_view();

    // Create the UI
    void setup();

    // Set UI elements
    void set_path(const std::string& path) const;

    // Removes all elements from the current view
    void clear_view(const std::function<void(void*)>& deleter = {}) const;

    // Fills the view from the given elements, applying the correct sorting
    void fill_view(std::vector<list_view_row> items) const;

    // Signals that a button has been clicked
    void button_clicked(view_button_type which) const;

    // Signals that the list view has been clicked in some way
    void list_clicked(NMHDR* nm);

    // Set the preview to the specified element
    void set_preview(std::unique_ptr<preview> preview) const;

    // Delete the current preview
    void clear_preview() const;

    // Constructs and executes the given context menu
    void execute_context_menu(const context_menu& menu, POINT pt) const;


    // Retrieve main window
    main_window* window() const;

    // Current selected column key
    data_key selected_column() const;
    sort_order selected_column_order() const;

    protected:
    nao_controller& controller;

    private:
    std::unique_ptr<main_window> _m_main_window;

    friend class preview;

    // Current column ordering of the view
    std::map<data_key, sort_order> _m_sort_order;
    data_key _m_selected_column;
};
