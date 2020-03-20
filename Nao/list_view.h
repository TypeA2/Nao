#pragma once

#include "ui_element.h"

#include <string>
#include <vector>
#include <functional>

#include "concepts.h"
#include "com.h"

class list_view : public virtual ui_element {
    com_ptr<IImageList> _image_list;

    public:
    enum sort_arrow {
        NoArrow,
        UpArrow,
        DownArrow
    };

    enum column_alignment {
        Left   = LVCFMT_LEFT,
        Center = LVCFMT_CENTER,
        Right  = LVCFMT_RIGHT
    };

    explicit list_view(ui_element* parent);
    explicit list_view(ui_element* parent,
        const std::vector<std::string>& hdr, const com_ptr<IImageList>& list = nullptr);
    list_view() = delete;

    ~list_view() override = default;
    void set_columns(const std::vector<std::string>& hdr) const;
    void set_image_list(const com_ptr<IImageList>& list);

    int column_count() const;
    int item_count() const;

    void get_item(LVITEMW& item) const;

    template <std::convertible_to<void*> T>
    T get_item_data(int index) const {
        return static_cast<T>(get_item_data(index));
    }

    void* get_item_data(int index) const;

    int add_item(const std::vector<std::string>& text, int image, void* extra = nullptr) const;

    int item_at(POINT pt) const;
    HWND header() const;

    void sort(PFNLVCOMPARE cb, LPARAM extra) const;

    template <concepts::pointer T>
    void sort(PFNLVCOMPARE cb, T extra) {
        sort(cb, reinterpret_cast<LPARAM>(extra));
    }

    void set_sort_arrow(int64_t col, sort_arrow direction) const;
    void set_column_width(int64_t col, int64_t width, int64_t min = 0) const;
    void set_column_alignment(int64_t col, column_alignment align) const;

    void clear(const std::function<void(void*)>& deleter = { }) const;

    int index_of(void* data) const;

    int selected() const;
    void* selected_data() const;

    void select(int64_t index) const;
    void select(void* data) const;
};

