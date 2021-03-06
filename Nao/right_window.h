#pragma once

#include "ui_element.h"
#include "nao_view.h"
#include "preview.h"

class right_window : public ui_element {
    preview_ptr _preview;

    public:
    explicit right_window(ui_element* parent);
    ~right_window() override = default;

    void set_preview(preview_ptr instance);
    void remove_preview();
    preview* get_preview() const;

    protected:
    void wm_size(int, const dimensions& dims) override;
};
