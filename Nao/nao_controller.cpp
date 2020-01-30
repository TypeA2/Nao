#include "nao_controller.h"
#include "utils.h"

#include "auto_wrapper.h"

#include <filesystem>

nao_controller::nao_controller()
    : view(*this), model(view, *this)
    , _m_worker(1,
                 [] { HASSERT(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE) == S_OK); },
                 [] { CoUninitialize(); })

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

void nao_controller::_handle_message(nao_thread_message msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        //// Begin model messages
        case TM_CONTENTS_CHANGED:
            _refresh_view();
            break;
        //// End model messages


        //// Begin view messages
        case TM_BUTTON_CLICKED:
            _handle_button_clicked(static_cast<view_button_type>(wparam));
            break;

        //// End view messages
        default:
            utils::coutln("thread message:", msg, wparam, lparam);
    }
    
}

void nao_controller::_refresh_view() const {
    view.set_path(model.current_path());

    view.clear_view();

    const item_provider_ptr& p = model.current_provider();
    const auto& data = p->data();

    std::vector<list_view_row> list_data(data.size());

    auto transform_func = [](const item_data& data) -> list_view_row {
        return {
            .name = data.name,
            .type = data.type,
            .size = (!data.dir && data.size_str.empty()) ? utils::bytes(data.size) : data.size_str,
            .compressed = (data.compression == 0.) ? "" : (std::to_string(int64_t(data.compression / 100.)) + '%'),
            .icon = data.icon,
            .data = &data
        };
    };
    std::transform(data.begin(), data.end(), list_data.begin(), transform_func);
    view.fill_view(list_data);
}

void nao_controller::_handle_button_clicked(view_button_type which) {
    switch (which) {
        case ViewButtonUp: {
            _m_worker.push(&nao_model::move_up, &model);
            break;
        }
    }
}
