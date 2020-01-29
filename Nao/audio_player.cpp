#include "audio_player.h"

#include "resource.h"

#include "utils.h"
#include "push_button.h"
#include "dimensions.h"
#include "binary_stream.h"

#include <mfapi.h>
#include <Mferror.h>
#include <Shlwapi.h>

ULONG audio_player::AddRef() {
    return InterlockedIncrement(&_m_refcount);
}

ULONG audio_player::Release() {
    auto count = InterlockedDecrement(&_m_refcount);

    if (count == 0) {
        delete this;
    }

    return count;
}

HRESULT audio_player::QueryInterface(const IID& riid, void** ppvObject) {
    static const QITAB qit[] = {
        QITABENT(audio_player, IMFAsyncCallback),
        { }
    };

    return QISearch(this, qit, riid, ppvObject);
}

HRESULT audio_player::GetParameters(DWORD* pdwFlags, DWORD* pdwQueue) {
    return E_NOTIMPL;
}

HRESULT audio_player::Invoke(IMFAsyncResult* pAsyncResult) {
    IMFMediaEvent* event = nullptr;
    ASSERT(SUCCEEDED(_m_session->EndGetEvent(pAsyncResult, &event)));

    MediaEventType type;
    ASSERT(SUCCEEDED(event->GetType(&type)));

    utils::coutln("Event of type:", type);

    if (type == MESessionClosed) {
        utils::coutln("close");
    }

    _m_session->BeginGetEvent(this, nullptr);

    if (_m_state != PlayerStateClosing) {
        event->AddRef();

        PostMessageW(handle(), WM_APP_MSG, WPARAM(event), LPARAM(type));
    }

    event->Release();

    return S_OK;
}

audio_player::audio_player(ui_element* parent)
    : ui_element(parent)
    , _m_refcount { 1 }
    , _m_is_playing { false } {
    _init();
}

audio_player::~audio_player() {
    DeleteObject(_m_play_icon);
    DeleteObject(_m_pause_icon);

    if (_m_source) {
        _m_source->Release();
    }

    if (_m_session) {
        _m_session->Release();
    }

    MFShutdown();
}

void audio_player::set_audio(const std::string& name,
    const std::shared_ptr<binary_istream>& stream) {
    int i = 1;
    _create_session();
    _create_source(name, stream);

    IMFPresentationDescriptor* source_pd = nullptr;
    ASSERT(SUCCEEDED(_m_source->CreatePresentationDescriptor(&source_pd)));

    IMFTopology* topology = _create_topology(source_pd);
    ASSERT(SUCCEEDED(_m_session->SetTopology(0, topology)));

    _m_state = PlayerStatePending;

    source_pd->Release();
    topology->Release();
}

HRESULT audio_player::pause() {
    if (_m_state != PlayerStateStarted) {
        return MF_E_INVALIDREQUEST;
    }

    if (!_m_session || !_m_source) {
        return E_UNEXPECTED;
    }

    HRESULT hr = _m_session->Pause();

    if (SUCCEEDED(hr)) {
        _m_state = PlayerStatePaused;
    }

    return hr;
}

HRESULT audio_player::play() {
    if (_m_state != PlayerStatePaused) {
        return MF_E_INVALIDREQUEST;
    }

    if (!_m_session || !_m_source) {
        return E_UNEXPECTED;
    }

    return _start_playback();
}



bool audio_player::wm_create(CREATESTRUCTW* create) {
    (void) create;
    HMODULE mmcndmgr = LoadLibraryW(L"mmcndmgr.dll");
    ASSERT(mmcndmgr);
    LoadIconWithScaleDown(mmcndmgr, MAKEINTRESOURCEW(30529),
        dims::play_button_size, dims::play_button_size, &_m_play_icon);
    LoadIconWithScaleDown(mmcndmgr, MAKEINTRESOURCEW(30531),
        dims::play_button_size, dims::play_button_size, &_m_pause_icon);
    FreeLibrary(mmcndmgr);

    RECT rect;
    GetWindowRect(handle(), &rect);
    _m_toggle = std::make_unique<push_button>(this, icon(_m_play_icon, false));

    return true;
}

void audio_player::wm_size(int type, int width, int height) {
    HDWP dwp = BeginDeferWindowPos(1);

    _m_toggle->move_dwp(dwp, ((width - 2 * dims::gutter_size) / 2) - (dims::play_button_size / 2),
        dims::gutter_size, dims::play_button_size, dims::play_button_size);

    EndDeferWindowPos(dwp);
}



void audio_player::_init() {
    ASSERT(MFStartup(MF_VERSION) == S_OK);

    HINSTANCE inst = GetModuleHandleW(nullptr);

    union {
        LPCWSTR str;
        WCHAR buf[sizeof(str) / sizeof(WCHAR)];
    } pun { };

    int length = LoadStringW(inst, IDS_AUDIO_PLAYER, pun.buf, 0);
    std::wstring class_name(length + 1i64, L'\0');
    wcsncpy_s(class_name.data(), length + 1i64, pun.str, length);

    // Check if the class has already been registered
    if (GetClassInfoExW(inst, class_name.c_str(), nullptr) == 0) {
        // Need to register it
        WNDCLASSEXW wcx {
            .cbSize        = sizeof(wcx),
            .style         = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc   = wnd_proc_fwd,
            .hInstance     = inst,
            .hCursor       = LoadCursorW(nullptr, IDC_ARROW),
            .hbrBackground = HBRUSH(COLOR_WINDOW + 1),
            .lpszClassName = class_name.c_str()
        };

        ASSERT(RegisterClassExW(&wcx) != 0);
    }

    RECT rect;
    GetClientRect(parent()->handle(), &rect);

    HWND handle = CreateWindowExW(0, class_name.c_str(), L"",
        WS_CHILD | WS_VISIBLE | SS_SUNKEN,
        0, 0, rect.right, rect.bottom,
        parent()->handle(), nullptr, inst,
        new wnd_init(this, &audio_player::_wnd_proc));

    ASSERT(handle);
}

void audio_player::_toggle_pause_play() {
    _m_is_playing = !_m_is_playing;

    _m_toggle->set_icon(icon(_m_is_playing ? _m_pause_icon : _m_play_icon, false));

    ASSERT(SUCCEEDED(_m_is_playing ? play() : pause()));
}

HRESULT audio_player::_start_playback() {
    ASSERT(_m_session);

    PROPVARIANT var;
    PropVariantInit(&var);

    HRESULT hr = _m_session->Start(&GUID_NULL, &var);

    if (SUCCEEDED(hr)) {
        _m_state = PlayerStateStarted;
    }

    PropVariantClear(&var);
    return hr;
}



void audio_player::_create_session() {
    _close_session();

    ASSERT(_m_state == PlayerStateClosed);

    ASSERT(SUCCEEDED(MFCreateMediaSession(nullptr, &_m_session)));

    ASSERT(SUCCEEDED(_m_session->BeginGetEvent(this, nullptr)));
}

void audio_player::_close_session() {

    HRESULT hr = S_OK;
    if (_m_session) {
        _m_state = PlayerStateClosing;

        hr = _m_session->Close();
        if (SUCCEEDED(hr)) {
            ASSERT(WaitForSingleObject(_m_close_event, 5000) != WAIT_TIMEOUT);
        }
    }

    if (SUCCEEDED(hr)) {
        if (_m_source) {
            _m_source->Shutdown();
        }

        if (_m_session) {
            _m_session->Shutdown();
        }
    }

    if (_m_source) {
        _m_source->Release();
    }

    if (_m_session) {
        _m_session->Release();
    }

    _m_state = PlayerStateClosed;
}



void audio_player::_create_source(const std::string& name,
    const std::shared_ptr<binary_istream>& stream) {
    

    IMFSourceResolver* resolver = nullptr;
    ASSERT(SUCCEEDED(MFCreateSourceResolver(&resolver)));


    IUnknown* unk = nullptr;
    MF_OBJECT_TYPE type = MF_OBJECT_INVALID;
    ASSERT(SUCCEEDED(resolver->CreateObjectFromByteStream(
        stream.get(),
        utils::utf16(name).c_str(),
        MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_KEEP_BYTE_STREAM_ALIVE_ON_FAIL,
        nullptr, &type, &unk)));

    ASSERT(SUCCEEDED(unk->QueryInterface(&_m_source)));

    resolver->Release();
    unk->Release();
}



IMFTopology* audio_player::_create_topology(IMFPresentationDescriptor* pd) {
    IMFTopology* topology = nullptr;

    ASSERT(SUCCEEDED(MFCreateTopology(&topology)));

    DWORD stream_count;
    ASSERT(SUCCEEDED(pd->GetStreamDescriptorCount(&stream_count)));

    auto create_sink = [](IMFStreamDescriptor* sd, IMFActivate** activate) {
        IMFMediaTypeHandler* handler = nullptr;

        ASSERT(SUCCEEDED(sd->GetMediaTypeHandler(&handler)));

        GUID major_type;
        ASSERT(SUCCEEDED(handler->GetMajorType(&major_type)));

        if (major_type == MFMediaType_Audio) {
            ASSERT(SUCCEEDED(MFCreateAudioRendererActivate(activate)));
        }

        handler->Release();
    };

    auto add_source = [this, pd, topology](IMFStreamDescriptor* sd, IMFTopologyNode** node) {
        ASSERT(SUCCEEDED(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, node)));
        ASSERT(SUCCEEDED((*node)->SetUnknown(MF_TOPONODE_SOURCE, _m_source)));
        ASSERT(SUCCEEDED((*node)->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pd)));
        ASSERT(SUCCEEDED((*node)->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, sd)));
        ASSERT(SUCCEEDED(topology->AddNode(*node)));
    };

    auto add_output = [topology] (IMFActivate* activate, IMFTopologyNode** node) {
        ASSERT(SUCCEEDED(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, node)));
        ASSERT(SUCCEEDED((*node)->SetObject(activate)));
        ASSERT(SUCCEEDED((*node)->SetUINT32(MF_TOPONODE_STREAMID, 0)));
        ASSERT(SUCCEEDED((*node)->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, false)));
        ASSERT(SUCCEEDED(topology->AddNode(*node)));
    };

    auto add_branch = [pd, create_sink, add_source, add_output](DWORD index) {
        IMFStreamDescriptor* sd = nullptr;
        BOOL selected = false;
        ASSERT(SUCCEEDED(pd->GetStreamDescriptorByIndex(index, &selected, &sd)));

        IMFActivate* activate = nullptr;
        IMFTopologyNode *source_node = nullptr, *output_node = nullptr;
        if (selected) {
            create_sink(sd, &activate);
            add_source(sd, &source_node);
            add_output(activate, &output_node);

            source_node->ConnectOutput(0, output_node, 0);
        }

        sd->Release();
        activate->Release();
        source_node->Release();
        output_node->Release();
    };

    for (DWORD i = 0; i < stream_count; ++i) {
        add_branch(i);
    }

    return topology;
}


template <class Q>
HRESULT GetEventObject(IMFMediaEvent* pEvent, Q** ppObject) {
    *ppObject = nullptr;   // zero output

    PROPVARIANT var;
    HRESULT hr = pEvent->GetValue(&var);
    if (SUCCEEDED(hr))
    {
        if (var.vt == VT_UNKNOWN)
        {
            hr = var.punkVal->QueryInterface(ppObject);
        } else
        {
            hr = MF_E_INVALIDTYPE;
        }
        PropVariantClear(&var);
    }
    return hr;
}


LRESULT audio_player::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_COMMAND: {
            HWND target = HWND(lparam);

            if (target == _m_toggle->handle()) {
                _toggle_pause_play();
            }

            break;
        }

        case WM_APP_MSG:
            return _player_event(wparam, lparam);

        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return 0;
}

LRESULT audio_player::_player_event(WPARAM wparam, LPARAM lparam) {
    IMFMediaEvent* event = reinterpret_cast<IMFMediaEvent*>(wparam);

    if (!event) {
        return E_POINTER;
    }

    MediaEventType type = static_cast<MediaEventType>(lparam);

    HRESULT status = 0;
    HRESULT hr = event->GetStatus(&status);

    if (SUCCEEDED(hr) && FAILED(status)) {
        hr = status;
    }

    ASSERT(SUCCEEDED(hr));

    switch (type) {
        case MESessionTopologyStatus: {
            UINT32 _status;
            ASSERT(SUCCEEDED(event->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &_status)));

            if (_status == MF_TOPOSTATUS_READY) {
                _m_state = PlayerStatePaused;
            }
            break;
        }

        case MEEndOfPresentation:
            _m_state = PlayerStateStopped;
            break;

        case MENewPresentation: {
            IMFPresentationDescriptor* pd = nullptr;
            ASSERT(SUCCEEDED(GetEventObject(event, &pd)));

            IMFTopology* topology = _create_topology(pd);
            ASSERT(SUCCEEDED(_m_session->SetTopology(0, topology)));

            _m_state = PlayerStatePending;

            topology->Release();
            pd->Release();
            break;
        }

        default: break;
    }

    event->Release();
    return S_OK;
}

