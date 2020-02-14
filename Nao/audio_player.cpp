#include "audio_player.h"

#include "utils.h"

#include "nao_controller.h"

audio_player::audio_player(const istream_ptr& stream, const std::string& path, nao_controller& controller)
    : controller(controller), _m_ref_count { 1 }, _m_stream { stream }, _m_path { path }
    , _m_playback_state { STATE_STOPPED } {
    ASSERT(MFStartup(MF_VERSION) == S_OK);

    HASSERT(MFCreateMediaSession(nullptr, &_m_session));
    
    HASSERT(_m_session->BeginGetEvent(this, nullptr));

    IMFSourceResolver* resolver = nullptr;
    HASSERT(MFCreateSourceResolver(&resolver));

    IUnknown* unk = nullptr;
    MF_OBJECT_TYPE type = MF_OBJECT_INVALID;
    HASSERT(resolver->CreateObjectFromByteStream(
        _m_stream.get(), utils::utf16(_m_path).c_str(),
        MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_KEEP_BYTE_STREAM_ALIVE_ON_FAIL,
        nullptr, &type, &unk));

    HASSERT(unk->QueryInterface(&_m_source));

    resolver->Release();
    unk->Release();

    IMFPresentationDescriptor* source_pd = nullptr;
    HASSERT(_m_source->CreatePresentationDescriptor(&source_pd));

    IMFTopology* topology = _create_topology(source_pd);
    HASSERT(_m_session->SetTopology(0, topology));

    _m_playback_state = STATE_PENDING;

    {
        std::unique_lock lock(_m_mutex);
        _m_condition.wait(lock, [this] { return _m_volume != nullptr; });
    }

    DWORD caps;
    _m_session->GetSessionCapabilities(&caps);
    ASSERT(caps & MFSESSIONCAP_SEEK);

    int64_t duration;
    HASSERT(source_pd->GetUINT64(MF_PD_DURATION, reinterpret_cast<uint64_t*>(&duration)));

    using namespace std::chrono_literals;
    _m_duration = duration * 100ns;

    {
        uint32_t length;
        HASSERT(source_pd->GetStringLength(MF_PD_MIME_TYPE, &length));

        std::wstring tmp(length, L'\0');
        HASSERT(source_pd->GetString(MF_PD_MIME_TYPE, tmp.data(), length + 1, nullptr));
        _m_mime_type = utils::utf8(tmp);
    }

    HASSERT(source_pd->GetUINT32(MF_PD_AUDIO_ENCODING_BITRATE, &_m_bitrate));

    source_pd->Release();
    topology->Release();

    IMFClock* _clock;
    HASSERT(_m_session->GetClock(&_clock));
    HASSERT(_clock->QueryInterface(&_m_clock));
    _clock->Release();

    _m_clock->AddClockStateSink(this);
}

audio_player::~audio_player() {
    HRESULT hr = S_OK;
    if (_m_session) {
        _m_playback_state = STATE_CLOSING;

        hr = _m_session->Close();
    }

    if (SUCCEEDED(hr)) {
        if (_m_source) {
            _m_source->Shutdown();
        }

        if (_m_session) {
            _m_session->Shutdown();
        }
    }

    if (_m_session) {
        _m_session->Release();
    }

    if (_m_source) {
        _m_source->Release();
    }

    if (_m_volume) {
        _m_volume->Release();
    }

    if (_m_clock) {
        _m_clock->RemoveClockStateSink(this);
        _m_clock->Release();
    }

    MFShutdown();
}

void audio_player::toggle_playback() {
    ASSERT(_m_session && _m_source);
    switch (_m_playback_state) {
        case STATE_STOPPED:
        case STATE_PAUSED: {
            PROPVARIANT var;
            PropVariantInit(&var);

            HRESULT hr = _m_session->Start(&GUID_NULL, &var);

            if (SUCCEEDED(hr)) {
                _m_playback_state = STATE_PLAYING;
            }

            PropVariantClear(&var);
            HASSERT(hr);

            break;
        }

        case STATE_PLAYING: {
            HASSERT(_m_session->Pause());
            _m_playback_state = STATE_PAUSED;
            break;
        }

        default: break;
    }
}

playback_state audio_player::state() const {
    return _m_playback_state;
}

void audio_player::set_volume_scaled(float val) const {
    HASSERT(_m_volume->SetMasterVolume(val));
}

void audio_player::set_volume_log(float orig, float curve) const {
    set_volume_scaled(pow(orig, curve));
}

float audio_player::get_volume_scaled() const {
    float volume = 0.f;
    HASSERT(_m_volume->GetMasterVolume(&volume));

    return volume;
}

float audio_player::get_volume_log(float curve) const {
    return pow(get_volume_scaled(), 1 / curve);
}

std::chrono::nanoseconds audio_player::get_duration() const {
    return _m_duration;
}

std::chrono::nanoseconds audio_player::get_current_time() const {
    MFTIME time;
    HASSERT(_m_clock->GetTime(&time));
    
    return time * std::chrono::nanoseconds(100);
}

const std::string& audio_player::get_mime_type() const {
    return _m_mime_type;
}

uint32_t audio_player::get_bitrate() const {
    return _m_bitrate;
}

void audio_player::seek(std::chrono::nanoseconds to, bool resume) {
    ASSERT(to >= std::chrono::nanoseconds(0) && to <= _m_duration);
    ASSERT(_m_playback_state == STATE_PAUSED);
    utils::coutln("seek to", to.count() / 1e6, "ms");
    PROPVARIANT var {
        .vt = VT_I8,
        .hVal = LARGE_INTEGER {
            .QuadPart = to.count() / 100
        }
    };

    HASSERT(_m_session->Start(&GUID_NULL, &var));
    _m_playback_state = STATE_PLAYING;

    if (!resume) {
        toggle_playback();
    }
}

void audio_player::add_event(event_type type, const std::function<void()>& func) {
    if (func) {
        _m_event_handlers[type].push_back(func);
    }
}

void audio_player::trigger_event(event_type type) const {
    if (_m_event_handlers.find(type) == _m_event_handlers.end()) {
        return;
    }

    for (const auto& func : _m_event_handlers.at(type)) {
        func();
    }
}

void audio_player::_handle_event(IMFMediaEvent* event, MediaEventType type) {
    if (!event) {
        return /* E_POINTER */;
    }

    HRESULT status = 0;
    HASSERT(event->GetStatus(&status));

    HASSERT(status);
    
    switch (type) {
        case MESessionTopologyStatus: {
            // Ready status?
            UINT32 _status;
            HASSERT(event->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &_status));
            if (_status == MF_TOPOSTATUS_READY) {
                _m_playback_state = STATE_PAUSED;

                HASSERT(MFGetService(_m_session, MR_POLICY_VOLUME_SERVICE, IID_PPV_ARGS(&_m_volume)));
                _m_condition.notify_one();
            }
            break;
        }

        case MEEndOfPresentation:
            _m_playback_state = STATE_STOPPED;
            break;

        case MENewPresentation: {
            IMFPresentationDescriptor* pd = nullptr;

            PROPVARIANT var;
            HASSERT(event->GetValue(&var));
            ASSERT(var.vt == VT_UNKNOWN);

            var.punkVal->QueryInterface(&pd);
            PropVariantClear(&var);

            IMFTopology* topology = _create_topology(pd);
            HASSERT(_m_session->SetTopology(0, topology));

            _m_playback_state = STATE_PENDING;

            topology->Release();
            pd->Release();
            break;
        }

        default: break;
    }

    event->Release();
}

IMFTopology* audio_player::_create_topology(IMFPresentationDescriptor* pd) const {
    IMFTopology* topology = nullptr;

    HASSERT(MFCreateTopology(&topology));

    DWORD stream_count;
    HASSERT(pd->GetStreamDescriptorCount(&stream_count));

    for (DWORD i = 0; i < stream_count; ++i) {
        IMFStreamDescriptor* sd = nullptr;
        BOOL selected = false;
        HASSERT(pd->GetStreamDescriptorByIndex(i, &selected, &sd));

        IMFActivate* activate = nullptr;
        IMFTopologyNode* source_node = nullptr, * output_node = nullptr;
        if (selected) {
            // Create sink
            IMFMediaTypeHandler* handler = nullptr;

            HASSERT(sd->GetMediaTypeHandler(&handler));

            GUID major_type;
            HASSERT(handler->GetMajorType(&major_type));

            if (major_type == MFMediaType_Audio) {
                HASSERT(MFCreateAudioRendererActivate(&activate));
            }
            handler->Release();

            // Add source node
            HASSERT(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &source_node));
            HASSERT(source_node->SetUnknown(MF_TOPONODE_SOURCE, _m_source));
            HASSERT(source_node->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pd));
            HASSERT(source_node->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, sd));
            HASSERT(topology->AddNode(source_node));
            
            // Add output node
            HASSERT(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &output_node));
            HASSERT(output_node->SetObject(activate));
            HASSERT(output_node->SetUINT32(MF_TOPONODE_STREAMID, 0));
            HASSERT(output_node->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, false));
            HASSERT(topology->AddNode(output_node));

            source_node->ConnectOutput(0, output_node, 0);
        }

        sd->Release();
        activate->Release();
        source_node->Release();
        output_node->Release();
    }

    return topology;
}

ULONG audio_player::AddRef() {
    return InterlockedIncrement(&_m_ref_count);
}

ULONG audio_player::Release() {
    auto count = InterlockedDecrement(&_m_ref_count);

    if (count == 0) {
        delete this;
    }

    return count;
}

HRESULT audio_player::QueryInterface(const IID& riid, void** ppvObject) {
    static const QITAB qit[] = {
        QITABENT(audio_player, IMFAsyncCallback),
        QITABENT(audio_player, IAudioEndpointVolumeCallback),
        QITABENT(audio_player, IMFClockStateSink),
        { }
    };

    return QISearch(this, qit, riid, ppvObject);
}

HRESULT audio_player::GetParameters(DWORD*, DWORD*) {
    return E_NOTIMPL;
}

HRESULT audio_player::Invoke(IMFAsyncResult* pAsyncResult) {
    IMFMediaEvent* event = nullptr;
    if (FAILED(_m_session->EndGetEvent(pAsyncResult, &event))) {
        return S_OK;
    }
    
    MediaEventType type = MEUnknown;
    HASSERT(event->GetType(&type));

    if (FAILED(_m_session->BeginGetEvent(this, nullptr))) {
        event->Release();
        return S_OK;
    }

    if (_m_playback_state != STATE_CLOSING) {
        event->AddRef();

        if (type == MESessionTopologyStatus) {
            // Don't process this setup message on the main thread
            _handle_event(event, type);
        } else {
            auto func = new std::function<void()>(std::bind(&audio_player::_handle_event, this, event, type));

            controller.post_message(TM_EXECUTE_FUNC, 0, func);
        }
    }

    event->Release();
    return S_OK;
}

HRESULT audio_player::OnClockPause(MFTIME) {
    trigger_event(EVENT_PAUSE);
    return S_OK;
}

HRESULT audio_player::OnClockRestart(MFTIME) {
    trigger_event(EVENT_RESTART);
    return S_OK;
}

HRESULT audio_player::OnClockSetRate(MFTIME, float) {
    trigger_event(EVENT_SET_RATE);
    return S_OK;
}

HRESULT audio_player::OnClockStart(MFTIME, LONGLONG) {
    trigger_event(EVENT_START);
    return S_OK;
}

HRESULT audio_player::OnClockStop(MFTIME) {
    trigger_event(EVENT_STOP);
    return S_OK;
}

IAudioEndpointVolume* audio_player::_default_endpoint_volume { };
