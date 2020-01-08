#pragma once

#include "ui_element.h"

#include <string>
#include <vector>

class list_view : public ui_element {
    public:
    explicit list_view(ui_element* parent);
    list_view(ui_element* parent, const std::vector<std::string>& hdr, IImageList* list = nullptr);
    list_view() = delete;

    ~list_view();
    
    void set_columns(const std::vector<std::string>& hdr);
    void set_image_list(IImageList* list);

    private:
    // Create window
    void _init();

    // Column count
    int _m_cols;

    IImageList* _m_image_list;
};

