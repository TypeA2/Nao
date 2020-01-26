#pragma once

#include "frameworks.h"

#include <vector>
#include <string>
#include <memory>

class nao_controller;
class main_window;

class nao_view {
    public:

    enum sort_order {
        SortOrderNone,
        SortOrderNormal,
        SortOrderReverse
    };

    static const std::vector<std::string>& list_view_header();
    static const std::vector<sort_order>& list_view_default_sort();
    static IImageList* shell_image_list();

    explicit nao_view(nao_controller& controller);
    nao_view() = delete;
    ~nao_view();

    // Create the UI
    void setup();

    protected:
    nao_controller& controller;

    private:
    std::unique_ptr<main_window> _m_main_window;

    public:
    enum element_type {
        ElementMainWindow,
        ElementRightWindow,
        ElementMainListView,
        ElementPathEdit,
        ElementDirectoryUpButton
    };

};

