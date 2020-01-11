#include "list_view.h"

#include "utils.h"

list_view::list_view(ui_element* parent) : ui_element(parent) {
    // This element can only be a child
    ASSERT(parent && parent->handle());
    _init();
}

list_view::list_view(ui_element* parent,
    const std::vector<std::string>& hdr, IImageList* list): list_view(parent) {
    set_columns(hdr);
    set_image_list(list);
}

list_view::~list_view() {
    _m_image_list->Release();
}

void list_view::set_columns(const std::vector<std::string>& hdr) {
    ASSERT(handle());
    ASSERT(hdr.size() <= std::numeric_limits<int>::max());

    _m_cols = int(hdr.size());

    LVCOLUMNW col;
    col.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
    col.cx = width() / _m_cols;
    col.fmt = LVCFMT_LEFT;
    int i = 0;

    for (const std::string& header : hdr) {
        col.iOrder = i++;
        
        std::wstring wide = utils::utf16(header);
        col.pszText = wide.data();

        if (col.iOrder == (_m_cols - 1)) {
            // Pad with last item
            col.cx = (width() + (_m_cols - 1)) / _m_cols;
        }

        ListView_InsertColumn(handle(), col.iOrder, &col);
    }
}

void list_view::set_image_list(IImageList* list) {
    ASSERT(list && handle());

    _m_image_list = list;
    
    ListView_SetImageList(handle(), list, LVSIL_SMALL);
}

void list_view::_init() {
    HINSTANCE inst = GetModuleHandleW(nullptr);
    
    HWND handle = CreateWindowExW(0,
        WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE |
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, 360, 360,
        parent()->handle(), nullptr, inst,
        nullptr);

    ASSERT(handle);

    SetWindowTheme(handle, L"Explorer", nullptr);
    ListView_SetExtendedListViewStyle(handle, LVS_EX_FULLROWSELECT);
    
    set_handle(handle);

}
