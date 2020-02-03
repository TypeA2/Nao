#include "nao_controller.h"
#include "utils.h"

#include "auto_wrapper.h"

#include <filesystem>

nao_controller::nao_controller()
    : view(*this), model(view, *this)
    , _m_worker(1,
                 [] { HASSERT(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)); },
                 CoUninitialize)

    , _m_main_threadid(GetCurrentThreadId()) {

    view.setup();

    _m_worker.push(&nao_model::setup, &model);
}
    
int nao_controller::pump() {
    (void) this;
    if (GetCurrentThreadId() != _m_main_threadid) {
        throw std::runtime_error("message pump called from outside main thread");
    }
    
    // Event loop
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        if (!msg.hwnd && msg.message > TM_FIRST && msg.message < TM_LAST) {
            _handle_message(static_cast<nao_thread_message>(msg.message), msg.wParam, msg.lParam);

            continue;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

DWORD nao_controller::main_threadid() const {
    return _m_main_threadid;
}

void nao_controller::clicked(click_event which) {
    switch (which) {
        case CLICK_MOVE_UP:
            _m_worker.push(&nao_model::move_up, &model);
            break;

        default: throw std::runtime_error("unsupported argumentless event " + std::to_string(which));
    }
}

void nao_controller::clicked(click_event which, void* arg) {
    switch (which) {
        case CLICK_DOUBLE_ITEM:
            _m_worker.push(&nao_model::move_down, &model, static_cast<item_data*>(arg));
            break;

        case CLICK_SINGLE_ITEM:
            _m_worker.push(&nao_model::fetch_preview, &model, static_cast<item_data*>(arg));
            break;

        default: throw std::runtime_error("unsupported void* argument event " + std::to_string(which));
    }
}

void nao_controller::list_view_preview_clicked(click_event which, void* arg) {
    switch (which) {
        case CLICK_DOUBLE_ITEM:
            _m_worker.push(&nao_model::move_down, &model, static_cast<item_data*>(arg));
            break;

        default: throw std::runtime_error("unsupported void* argument event " + std::to_string(which));
    }
}

int nao_controller::order_items(void* first, void* second, data_key key, sort_order order) {
    item_data* item1 = reinterpret_cast<item_data*>(first);
    item_data* item2 = reinterpret_cast<item_data*>(second);

    if (!item1 || !item2) {
        return 0;
    }

    int first1 = 0;
    int first2 = 0;

    switch (order) {
        case ORDER_NORMAL:
            first1 = -1;
            first2 = 1;
            break;

        case ORDER_REVERSE:
            first1 = 1;
            first2 = -1;
            break;

        default:
            break;
    }

    // Directories on top
    if (!item1->dir != !item2->dir) {
        return item1->dir ? -1 : 1;
    }

    const auto& f = std::use_facet<std::ctype<std::string::value_type>>(std::locale());

    auto cmp = [&, first1, first2](const std::string& left, const std::string& right) -> int {
        return std::lexicographical_compare(
            left.begin(), left.end(), right.begin(), right.end(),
            [&f](std::string::value_type a, std::string::value_type b) {
                return f.tolower(a) < f.tolower(b);
            }) ? first1 : first2;
    };

    switch (key) {
        case KEY_NAME: // Name, alphabetically
            if (item1->name == item2->name) { return 0; }

            return cmp(item1->name, item2->name);
        case KEY_TYPE: { // Type, alphabetically
            if (item1->type == item2->type) {
                // Fallback on name
                return cmp(item1->name, item2->name);
            }

            return cmp(item1->type, item2->type); }

        case KEY_SIZE: // File size
            if (item1->size == item2->size) {
                // Fallback on name
                return cmp(item1->name, item2->name);
            }

            return (item1->size < item2->size) ? first1 : first2;

        case KEY_COMP: // Compression ratio
            if (item1->compression == item2->compression) {
                // Fallback on name
                return cmp(item1->name, item2->name);
            }

            return (item1->compression < item2->compression) ? first1 : first2;

        default: return 0;
    }
}


void nao_controller::_handle_message(nao_thread_message msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        //// Begin model messages
        case TM_CONTENTS_CHANGED:
            _refresh_view();
            break;

        case TM_PREVIEW_CHANGED:
            _refresh_preview();
            break;

        //// End model messages

        default:
            utils::coutln("thread message:", msg, wparam, lparam);
    }
}

void nao_controller::_refresh_view() {
    (void) this;

    view.set_path(model.current_path());

    view.clear_view();
    view.clear_preview();

    const item_provider_ptr& p = model.current_provider();
    view.fill_view(_transform_data_to_row(p->data()));
}

void nao_controller::_refresh_preview() {
    const item_provider_ptr& p = model.preview_provider();

    if (p) {
        view.create_preview(PREVIEW_LIST_VIEW);

        view.list_view_preview_fill(_transform_data_to_row(p->data()));
    }
}

list_view_row nao_controller::_transform_data_to_row(const item_data& data) {
    return {
        .name = data.name,
        .type = data.type,
        .size = (!data.dir && data.size_str.empty()) ? utils::bytes(data.size) : data.size_str,
        .compressed = (data.compression == 0.) ? "" : (std::to_string(int64_t(data.compression / 100.)) + '%'),
        .icon = data.icon,
        .data = &data
    };
}

std::vector<list_view_row> nao_controller::_transform_data_to_row(const std::vector<item_data>& data) {
    std::vector<list_view_row> list_data(data.size());

    std::transform(data.begin(), data.end(), list_data.begin(),
        static_cast<list_view_row(*)(const item_data&)>(_transform_data_to_row));

    return list_data;
}

