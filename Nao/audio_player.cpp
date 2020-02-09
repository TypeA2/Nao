#include "audio_player.h"

#include "utils.h"

#include "nao_controller.h"
#include <bitset>

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

    source_pd->Release();
    topology->Release();
}

audio_player::~audio_player() {
    HRESULT(_default_endpoint_volume->UnregisterControlChangeNotify(this));

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

    MFShutdown();
}

void audio_player::toggle_playback() {
    ASSERT(_m_session && _m_source);
    switch (_m_playback_state) {
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
        { }
    };

    return QISearch(this, qit, riid, ppvObject);
}

HRESULT audio_player::GetParameters(DWORD*, DWORD*) {
    return E_NOTIMPL;
}

HRESULT audio_player::Invoke(IMFAsyncResult* pAsyncResult) {
    IMFMediaEvent* event = nullptr;
    HASSERT(_m_session->EndGetEvent(pAsyncResult, &event));

    MediaEventType type;
    HASSERT(event->GetType(&type));

    _m_session->BeginGetEvent(this, nullptr);

    if (_m_playback_state != STATE_CLOSING) {
        event->AddRef();

        auto func = new std::function<void()>(std::bind(&audio_player::_handle_event, this, event, type));

        controller.post_message(TM_EXECUTE_FUNC, 0, func);
    }

    event->Release();
    return S_OK;
}

HRESULT audio_player::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) {
    utils::coutln("notify", pNotify->bMuted, pNotify->fMasterVolume, pNotify->nChannels);

    _m_volume_scalar = pNotify->fMasterVolume;

    return S_OK;
}

void audio_player::set_volume_percent(float val) const {
    HASSERT(_m_volume->SetMasterVolume(val * _m_volume_scalar));
}

void audio_player::_handle_event(IMFMediaEvent* event, MediaEventType type) {
    if (!event) {
        return /* E_POINTER */;
    }

    HRESULT status = 0;
    HRESULT hr = event->GetStatus(&status);

    HASSERT(hr);
    HASSERT(status);

    switch (type) {
        case MESessionTopologyStatus: {
            UINT32 _status;
            HASSERT(event->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &_status));

            if (_status == MF_TOPOSTATUS_READY) {
                _m_playback_state = STATE_PAUSED;

                HASSERT(MFGetService(_m_session, MR_POLICY_VOLUME_SERVICE, IID_PPV_ARGS(&_m_volume)));

                // Get default volume if needed
                if (!_default_endpoint_volume) {
                    IMMDeviceEnumerator* enumerator = nullptr;
                    HASSERT(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&enumerator)));

                    IMMDevice* device = nullptr;
                    HASSERT(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device));
                    HASSERT(device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                        reinterpret_cast<void**>(&_default_endpoint_volume)));

                    device->Release();
                    enumerator->Release();
                } else {
                    _default_endpoint_volume->AddRef();
                }

                HASSERT(_default_endpoint_volume->RegisterControlChangeNotify(this));
                HASSERT(_default_endpoint_volume->GetMasterVolumeLevelScalar(&_m_volume_scalar));
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

        case MEAudioSessionVolumeChanged: {
            utils::coutln("volume changed");
            break;
        }

        default: break;
    }

    event->Release();
}

IMFTopology* audio_player::_create_topology(IMFPresentationDescriptor* pd) {
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

IAudioEndpointVolume* audio_player::_default_endpoint_volume { };
