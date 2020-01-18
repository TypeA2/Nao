#include "list_view.h"

#include "utils.h"

#include <algorithm>

list_view::list_view(ui_element* parent) : ui_element(parent) {
    // This element can only be a child
    ASSERT(parent && parent->handle());
    _init();
}

list_view::list_view(ui_element* parent,
    const std::vector<std::string>& hdr, IImageList* list) : list_view(parent) {
    set_columns(hdr);

    if (list) {
        set_image_list(list);
    }
}

list_view::~list_view() {
    _m_image_list->Release();
}

void list_view::set_columns(const std::vector<std::string>& hdr) const {
    ASSERT(handle());
    ASSERT(!hdr.empty() && hdr.size() <= std::numeric_limits<int>::max());

    int columns = int(hdr.size());

    LVCOLUMNW col;
    col.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH | LVCF_MINWIDTH;
    col.cx = width() / columns;
    col.cxMin = col.cx;
    col.fmt = LVCFMT_LEFT;
    int i = 0;

    for (const std::string& header : hdr) {
        col.iOrder = i++;
        
        std::wstring wide = utils::utf16(header);
        col.pszText = wide.data();

        if (col.iOrder == (columns - 1)) {
            // Pad with last item
            col.cx = (width() + (columns - 1)) / columns;
            col.cxMin = col.cx;
        }

        ListView_InsertColumn(handle(), col.iOrder, &col);
    }
}

void list_view::set_image_list(IImageList* list) {
    ASSERT(list && handle());

    _m_image_list = list;
    
    ListView_SetImageList(handle(), list, LVSIL_SMALL);
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
    LVITEMW item { };
    item.mask = LVIF_PARAM;
    item.iItem = index;
    ListView_GetItem(handle(), &item);

    return reinterpret_cast<void*>(item.lParam);
}




int list_view::add_item(const std::vector<std::string>& text, int image, LPARAM extra) const {
    std::vector<std::wstring> wide;
    std::transform(text.begin(), text.end(), std::back_inserter(wide), utils::utf16);

    return add_item(wide, image, extra);
}

int list_view::add_item(const std::vector<std::wstring>& text, int image, LPARAM extra) const {
    ASSERT(_m_image_list);
    ASSERT(!text.empty());
    size_t header_size = Header_GetItemCount(ListView_GetHeader(handle()));
    ASSERT(text.size() == header_size);

    LVITEMW item { };
    item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    item.lParam = extra;
    item.iItem = ListView_GetItemCount(handle());
    item.iImage = image;
    item.pszText = const_cast<LPWSTR>(text.front().data());

    ListView_InsertItem(handle(), &item);
    for (size_t i = 1; i < text.size(); ++i) {
        ListView_SetItemText(handle(), item.iItem, int(i), const_cast<LPWSTR>(text[i].data()));
    }

    return item.iItem;
}

int list_view::item_at(POINT pt) const {
    LVHITTESTINFO info { };
    info.pt = pt;

    ListView_SubItemHitTest(handle(), &info);

    return info.iItem;
}


void list_view::sort(int (CALLBACK* cb)(LPARAM, LPARAM, LPARAM), LPARAM extra) const {
    ListView_SortItems(handle(), cb, extra);
}

void list_view::set_sort_arrow(int col, sort_arrow direction) const {
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

void list_view::set_column_width(int col, int width, int min) const {
    ListView_SetColumnWidth(handle(), col, width);

    if (width == LVSCW_AUTOSIZE) {
        // Fix min width
        if (ListView_GetColumnWidth(handle(), col) < min) {
            ListView_SetColumnWidth(handle(), col, min);
        }
    }
}

void list_view::set_column_alignment(int col, column_alignment align) const {
    LVCOLUMNW c { };
    c.mask = LVCF_FMT;
    c.fmt = align;
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



void list_view::_init() {
    HINSTANCE inst = GetModuleHandleW(nullptr);
    
    HWND handle = CreateWindowExW(0,
        WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE |
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, parent()->width(), parent()->height(),
        parent()->handle(), nullptr, inst,
        nullptr);

    ASSERT(handle);

    SetWindowTheme(handle, L"Explorer", nullptr);
    ListView_SetExtendedListViewStyle(handle, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    
    set_handle(handle);
}
