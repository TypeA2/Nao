#include "nao_view.h"

#include "main_window.h"

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

