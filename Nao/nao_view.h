#pragma once

#include "frameworks.h"

#include <vector>
#include <string>
#include <memory>
#include <functional>

class nao_controller;
class main_window;

struct list_view_row {
    std::string name;
    std::string type;
    std::string size;
    std::string compressed;
    int icon;
    void const* data;
};

enum view_button_type {
    BUTTON_UP
};

enum sort_order {
    ORDER_NONE,
    ORDER_NORMAL,
    ORDER_REVERSE
};

class nao_view {
    public:
    static const std::vector<std::string>& list_view_header();
    static const std::vector<sort_order>& list_view_default_sort();
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
    void fill_view(const std::vector<list_view_row>& items) const;

    // Signals that a button has been clicked
    void button_clicked(view_button_type which) const;

    // Signals that the list view has been clicked in some way
    void list_clicked(NMHDR* nm) const;

    protected:
    nao_controller& controller;

    private:
    std::unique_ptr<main_window> _m_main_window;
};

