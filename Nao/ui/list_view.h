#pragma once

#include "ui_element.h"

#include <string>
#include <vector>

class list_view : public ui_element {
    public:
    enum sort_arrow {
        NoArrow,
        UpArrow,
        DownArrow
    };

    explicit list_view(ui_element* parent);
    list_view(ui_element* parent, const std::vector<std::string>& hdr, IImageList* list = nullptr);
    list_view() = delete;

    ~list_view();
    
    void set_columns(const std::vector<std::string>& hdr);
    void set_image_list(IImageList* list);

    int column_count() const;

    int add_item(const std::vector<std::string>& text,
        int image, LPARAM extra = 0) const;
    int add_item(const std::vector<std::wstring>& text,
        int image, LPARAM extra = 0) const;

    void sort(int(CALLBACK* cb)(LPARAM, LPARAM, LPARAM), LPARAM extra) const;

    void set_sort_arrow(int col, sort_arrow direction) const;

    private:
    
    // Create window
    void _init();

    // Column count
    int _m_cols;

    IImageList* _m_image_list;
};

