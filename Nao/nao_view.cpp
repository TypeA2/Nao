#include "nao_view.h"

#include "nao_controller.h"

#include "main_window.h"
#include "left_window.h"

#include "line_edit.h"
#include "push_button.h"
#include "list_view.h"

#include "utils.h"

const std::vector<std::string>& nao_view::list_view_header() {
    static std::vector<std::string> vec { "Name", "Type", "Size", "Compressed" };

    return vec;
}

const std::vector<nao_view::sort_order>& nao_view::list_view_default_sort() {
    static std::vector<sort_order> vec { SortOrderNormal, SortOrderNormal, SortOrderReverse, SortOrderReverse };

    return vec;
}

IImageList* nao_view::shell_image_list() {
    static IImageList* imglist = nullptr;

    if (!imglist) {
        if (FAILED(SHGetImageList(SHIL_SMALL, IID_PPV_ARGS(&imglist)))) {
            utils::coutln("failed to retrieve main image list");
            return nullptr;
        }

        return imglist;
    }

    // Increment ref count if already initialised
    imglist->AddRef();

    return imglist;
}


nao_view::nao_view(nao_controller& controller) : controller(controller) {
    
}

nao_view::~nao_view() {
    (void) this;
}


void nao_view::setup() {
    _m_main_window = std::make_unique<main_window>(this);
}

void nao_view::set_path(const std::string& path) const {
    auto left = _m_main_window->left();

    // Display the current path in the line edit above
    left->path()->set_text(path);

    // Only enable the up button whenever we are not at the root path
    left->view_up()->set_enabled(path != "\\");
}

void nao_view::clear_view(const std::function<void(void*)>& deleter) const {
    _m_main_window->left()->list()->clear(deleter);
}

void nao_view::fill_view(const std::vector<list_view_row>& items) const {
    if (items.empty()) {
        return;
    }

    list_view* list = _m_main_window->left()->list();

    for (const auto& [name, type, size,
        compressed, icon, data] : items) {

        list->add_item({ name, type, size, compressed }, icon, data);
    }
}

void nao_view::button_clicked(view_button_type which) const {
    controller.post_message(TM_BUTTON_CLICKED, 0, ViewButtonUp);
}
