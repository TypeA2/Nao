#include "list_view.h"

#include "utils.h"

#include "frameworks.h"

#include <algorithm>
#include <ranges>

#include <nao/strings.h>

static constexpr DWORD listview_style = win32::style | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS;

list_view::list_view(ui_element* parent)
    : ui_element(parent, WC_LISTVIEWW, parent->dims().rect(), listview_style) {

    SetWindowTheme(handle(), L"Explorer", nullptr);
    ListView_SetExtendedListViewStyle(handle(), LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
}

list_view::list_view(ui_element* parent, const std::vector<std::string>& hdr, const com_ptr<IImageList>& list) : list_view(parent) {
    set_columns(hdr);

    if (list) {
        set_image_list(list);
    }
}

void list_view::set_columns(const std::vector<std::string>& hdr) const {
    ASSERT(!hdr.empty() && hdr.size() <= std::numeric_limits<int>::max());

    int columns = utils::narrow<int>(hdr.size());
    int width = utils::narrow<int>(this->width());

    LVCOLUMNW col;
    col.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH | LVCF_MINWIDTH;
    col.cx = width / columns;
    col.cxMin = col.cx;
    col.fmt = LVCFMT_LEFT;
    int i = 0;
    
    for (const std::string& header : hdr) {
        col.iOrder = i++;
        
        std::wstring wide = nao::string { header }.wide().c_str();
        col.pszText = wide.data();

        if (col.iOrder == (columns - 1)) {
            // Pad with last item
            col.cx = (width + (columns - 1)) / columns;
            col.cxMin = col.cx;
        }

        ASSERT(ListView_InsertColumn(handle(), col.iOrder, &col) >= 0);
    }
}

void list_view::set_image_list(const com_ptr<IImageList>& list) {
    ASSERT(list);

    _image_list = list;
    
    ListView_SetImageList(handle(), list.GetInterfacePtr(), LVSIL_SMALL);
}

int list_view::column_count() const {
    return Header_GetItemCount(ListView_GetHeader(handle()));
}

int list_view::item_count() const {
    return ListView_GetItemCount(handle());
}

void list_view::get_item(LVITEMW& item) const  {
    ListView_GetItem(handle(), &item);
}

void* list_view::get_item_data(int index) const {
    LVITEMW item {
        .mask  = LVIF_PARAM,
        .iItem = index
    };
    ListView_GetItem(handle(), &item);

    return reinterpret_cast<void*>(item.lParam);
}

int list_view::add_item(const std::vector<std::string>& text, int image, void* extra) const {
    ASSERT(_image_list);
    ASSERT(!text.empty());
    ASSERT(utils::narrow<int>(text.size()) == column_count());

    std::vector<nao::wstring> utf16;
    std::transform(text.begin(), text.end(), std::back_inserter(utf16),
        [](const auto& str) { return nao::string { str }.wide(); });

    LVITEMW item { };
    item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    item.lParam = reinterpret_cast<LPARAM>(extra);
    item.iItem = ListView_GetItemCount(handle());
    item.iImage = image;
    item.pszText = utf16.front().data();

    ListView_InsertItem(handle(), &item);
    for (size_t i = 1; i < utf16.size(); ++i) {
        ListView_SetItemText(handle(), item.iItem, utils::narrow<int>(i), utf16[i].data());
    }

    return item.iItem;
}

int list_view::item_at(POINT pt) const {
    LVHITTESTINFO info { };
    info.pt = pt;

    ListView_SubItemHitTest(handle(), &info);

    return info.iItem;
}

HWND list_view::header() const {
    return ListView_GetHeader(handle());
}

void list_view::sort(PFNLVCOMPARE cb, LPARAM extra) const {
    ListView_SortItems(handle(), cb, extra);
}

void list_view::set_sort_arrow(int64_t col, sort_arrow direction) const {
    HWND header = ListView_GetHeader(handle());

    if (header) {
        HDITEMW hdr;
        hdr.mask = HDI_FORMAT;

        Header_GetItem(header, col, &hdr);

        switch (direction) {
            case NoArrow:
                hdr.fmt = (hdr.fmt & ~(HDF_SORTDOWN | HDF_SORTUP));
                break;
            case UpArrow:
                hdr.fmt = (hdr.fmt & ~HDF_SORTDOWN) | HDF_SORTUP;
                break;
            case DownArrow:
                hdr.fmt = (hdr.fmt & ~HDF_SORTUP) | HDF_SORTDOWN;
                break;
        }

        Header_SetItem(header, col, &hdr);
    }
}

void list_view::set_column_width(int64_t col, int64_t width, int64_t min) const {
    ListView_SetColumnWidth(handle(), col, width);

    if (width == LVSCW_AUTOSIZE) {
        // Fix min width
        if (ListView_GetColumnWidth(handle(), col) < min) {
            ListView_SetColumnWidth(handle(), col, min);
        }
    }
}

void list_view::set_column_alignment(int64_t col, column_alignment align) const {
    LVCOLUMNW c {
        .mask = LVCF_FMT,
        .fmt = align
    };

    ListView_SetColumn(handle(), col, &c);

    if (align != Left) {
        // Keep header left-aligned
        HWND header = ListView_GetHeader(handle());
        HDITEMW item { };
        item.mask = HDI_FORMAT;
        Header_GetItem(header, col, &item);

        // Zero alignment bits, which also left-aligns
        item.fmt &= ~HDF_JUSTIFYMASK;
        Header_SetItem(header, col, &item);
    }
}

void list_view::clear(const std::function<void(void*)>& deleter) const {
    if (deleter) {
        LVITEMW item { };
        item.mask = LVIF_PARAM;

        for (int i = 0; i < ListView_GetItemCount(handle()); ++i) {
            item.iItem = i;
            ListView_GetItem(handle(), &item);

            deleter(reinterpret_cast<void*>(item.lParam));
        }
    }

    ListView_DeleteAllItems(handle());
}

int list_view::index_of(void* data) const {
    LVFINDINFOW find {
        .flags = LVFI_PARAM,
        .lParam = reinterpret_cast<LPARAM>(data)
    };

    return ListView_FindItem(handle(), -1, &find);
}

int list_view::selected() const {
    return ListView_GetNextItem(handle(), -1, LVNI_SELECTED);
}

void* list_view::selected_data() const {
    return get_item_data(selected());
}

void list_view::select(int64_t index) const {
    ListView_SetItemState(handle(), index, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
}

void list_view::select(void* data) const {
    select(index_of(data));
}
