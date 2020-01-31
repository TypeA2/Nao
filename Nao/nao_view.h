#pragma once

#include "frameworks.h"
#include "ui_element.h"

#include <vector>
#include <string>

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

enum preview_element_type {
    PREVIEW_LIST_VIEW
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

    // Create the specified preview element
    void create_preview(preview_element_type type);

    // Delete the current preview
    void clear_preview() const;

    // If the preview is a list_view_preview, execute the same actions as on the main list_view
    void list_view_preview_fill(const std::vector<list_view_row>& items) const;
    void list_view_preview_clicked(NMHDR* nm) const;


    // Retrieve main window
    main_window* window() const;

    protected:
    nao_controller& controller;

    private:
    std::unique_ptr<main_window> _m_main_window;
};

// Wrapper class for preview elements
class preview : public ui_element {
    public:
    explicit preview(nao_view* view);

    protected:
    nao_view* view;
};

using preview_ptr = std::unique_ptr<preview>;

class list_view;
// Multi-use list-view preview
class list_view_preview : public preview {
    public:
    explicit list_view_preview(nao_view* view);
    ~list_view_preview() override = default;

    list_view* operator->() const;

    protected:
    bool wm_create(CREATESTRUCTW* create) override;

    private:
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    std::unique_ptr<list_view> _m_list;

    inline static bool _initialised = false;
};
