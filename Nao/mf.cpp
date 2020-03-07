#include "mf.h"

#include "com.h"
#include "win32.h"

namespace detail {
    class DECLSPEC_UUID("1E89C9C0-318E-4E1B-9EE1-81A586111507") async_result : public IUnknown {
        volatile uint32_t _refcount = 1;

        public:
        explicit async_result(BYTE * buf, ULONG count) : buf { buf }, count { count } { }
        virtual ~async_result() = default;

        STDMETHODIMP_(ULONG) AddRef() override {
            InterlockedIncrement(&_refcount);
            return _refcount;
        }
        STDMETHODIMP_(ULONG) Release() override {
            uint32_t refcount = InterlockedDecrement(&_refcount);

            if (_refcount == 0) {
                delete this;
            }

            return refcount;
        }

        STDMETHODIMP QueryInterface(const IID& riid, void** ppvObject) override {
            static const QITAB qit[] = {
                QITABENT(async_result, IUnknown),
                QITABENT(async_result, async_result),
                { }
            };

            return QISearch(this, qit, riid, ppvObject);
        }

        BYTE* buf;
        ULONG count;
        ULONG read = 0;

    };
}

namespace mf {
    HRESULT binary_stream_imfbytestream::QueryInterface(const IID& riid, void** ppvObject) {
        static const QITAB qit[] {
            QITABENT(binary_stream_imfbytestream, IMFByteStream),
            QITABENT(binary_stream_imfbytestream, IMFAsyncCallback), { }
        };
        
        return QISearch(this, qit, riid, ppvObject);
    }
    ULONG binary_stream_imfbytestream::AddRef() {
        return InterlockedIncrement(&_refcount);
    }

    ULONG binary_stream_imfbytestream::Release() {
        uint32_t count = InterlockedDecrement(&_refcount);

        if (count == 0) {
            delete this;
        }

        return count;
    }



    HRESULT binary_stream_imfbytestream::BeginRead(BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) {
        if (!_stream->good()) {
            return E_ABORT;
        }

        if (!pb || !pCallback) {
            return E_POINTER;
        }

        if (cb == 0) {
            return E_INVALIDARG;
        }

        com_ptr<IMFAsyncResult> result;
        HASSERT(MFCreateAsyncResult(new detail::async_result(pb, cb), pCallback, punkState, &result));
        HASSERT(MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, this, result));
        return S_OK;
    }

    HRESULT binary_stream_imfbytestream::EndRead(IMFAsyncResult* pResult, ULONG* pcbRead) {
        if (!pResult || !pcbRead) {
            return E_POINTER;
        }
        if (HRESULT hr = pResult->GetStatus(); FAILED(hr)) {
            return hr;
        }

        com_ptr<IUnknown> unk;
        HASSERT(pResult->GetObjectW(&unk));

        com_ptr<detail::async_result> res;
        HASSERT(unk->QueryInterface(&res));

        *pcbRead = res->read;

        return S_OK;
    }

    HRESULT binary_stream_imfbytestream::Read(BYTE* pb, ULONG cb, ULONG* pcbRead) {
        if (!pb) {
            return E_POINTER;
        }

        _stream->read(pb, cb);

        if (pcbRead) {
            *pcbRead = utils::narrow<LONG>(_stream->gcount());
        }

        return S_OK;
    }

    HRESULT binary_stream_imfbytestream::GetCapabilities(DWORD* pdwCapabilities) {
        if (!pdwCapabilities) {
            return E_POINTER;
        }

        *pdwCapabilities =
            MFBYTESTREAM_IS_READABLE |
            MFBYTESTREAM_IS_SEEKABLE |
            MFBYTESTREAM_DOES_NOT_USE_NETWORK;

        return S_OK;
    }

    HRESULT binary_stream_imfbytestream::GetCurrentPosition(QWORD* pqwPosition) {
        if (!pqwPosition) {
            return E_POINTER;
        }

        *pqwPosition = _stream->tellg();

        return S_OK;
    }

    HRESULT binary_stream_imfbytestream::GetLength(QWORD* pqwLength) {
        if (!pqwLength) {
            return E_POINTER;
        }

        auto cur = _stream->tellg();

        _stream->seekg(0, std::ios::end);

        *pqwLength = _stream->tellg();

        _stream->seekg(cur);

        return S_OK;
    }

    HRESULT binary_stream_imfbytestream::IsEndOfStream(BOOL* pfEndOfStream) {
        if (!pfEndOfStream) {
            return E_POINTER;
        }

        *pfEndOfStream = _stream->eof() ? TRUE : FALSE;

        return S_OK;
    }

    HRESULT binary_stream_imfbytestream::Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, DWORD, QWORD* pqwCurrentPosition) {
        switch (SeekOrigin) {
            case msoBegin:   _stream->seekg(llSeekOffset, std::ios::beg); break;
            case msoCurrent: _stream->seekg(llSeekOffset, std::ios::cur); break;
        }

        if (pqwCurrentPosition) {
            *pqwCurrentPosition = _stream->tellg();
        }

        return S_OK;
    }

    HRESULT binary_stream_imfbytestream::SetCurrentPosition(QWORD qwPosition) {
        _stream->seekg(qwPosition);
        return S_OK;
    }



    HRESULT binary_stream_imfbytestream::BeginWrite(const BYTE*, ULONG, IMFAsyncCallback*, IUnknown* ) {
        return E_NOTIMPL;
    }

    HRESULT binary_stream_imfbytestream::EndWrite(IMFAsyncResult*, ULONG*) {
        return E_NOTIMPL;
    }

    HRESULT binary_stream_imfbytestream::Write(const BYTE*, ULONG, ULONG*) {
        return E_NOTIMPL;
    }

    HRESULT binary_stream_imfbytestream::Close() {
        return S_OK;
    }

    HRESULT binary_stream_imfbytestream::Flush() {
        return E_NOTIMPL;
    }

    HRESULT binary_stream_imfbytestream::SetLength(QWORD) {
        return E_NOTIMPL;
    }

    HRESULT binary_stream_imfbytestream::GetParameters(DWORD*, DWORD*) {
        return E_NOTIMPL;
    }

    HRESULT binary_stream_imfbytestream::Invoke(IMFAsyncResult* pAsyncResult) {
        if (!_stream->good()) {
            return E_ABORT;
        }

        if (!pAsyncResult) {
            return E_POINTER;
        }

        com_ptr<IUnknown> unk_state;
        HASSERT(pAsyncResult->GetState(&unk_state));

        com_ptr<IMFAsyncResult> operation_result;
        HASSERT(unk_state->QueryInterface(&operation_result));

        com_ptr<IUnknown> unk_result;
        HASSERT(operation_result->GetObjectW(&unk_result));

        com_ptr<detail::async_result> result;
        HASSERT(unk_result->QueryInterface(&result));

        _stream->read(result->buf, result->count);

        if (_stream->eof()) {
            _stream->clear();
        }

        result->read = utils::narrow<ULONG>(_stream->gcount());


        if (operation_result) {
            operation_result->SetStatus((_stream->eof() || _stream->good()) ? S_OK : E_FAIL);

            MFInvokeCallback(operation_result);
        }

        return S_OK;
    }


    mf_interface::mf_interface() {
        HASSERT(MFStartup(MF_VERSION));
    }

    mf_interface::~mf_interface() {
        HASSERT(MFShutdown());
    }



    void display_control::repaint() const {
        com_object->RepaintVideo();
    }

    bool display_control::set_position(const rectangle& rect) const {
        RECT r {
            .left   = utils::narrow<LONG>(rect.x),
            .top    = utils::narrow<LONG>(rect.y),
            .right  = utils::narrow<LONG>(rect.width),
            .bottom = utils::narrow<LONG>(rect.height)
        };
        return SUCCEEDED(com_object->SetVideoPosition(nullptr, &r));
    }



    uint32_t attributes::get_uint32(const GUID& guid) const {
        uint32_t res;
        HASSERT(com_object->GetUINT32(guid, &res));

        return res;
    }



    MediaEventType media_event::type() const {
        MediaEventType type;
        HRESULT hr = com_object->GetType(&type);

        return SUCCEEDED(hr) ? type : MEError;
    }

    HRESULT media_event::status() const {
        HRESULT status;
        HRESULT hr = com_object->GetStatus(&status);

        return SUCCEEDED(hr) ? status : E_FAIL;
    }

    bool media_event::good() const {
        return SUCCEEDED(status());
    }

    attributes media_event::attributes() const {
        return mf::attributes { com_object };
    }



    media_session::media_session() {
        HASSERT(MFCreateMediaSession(nullptr, &com_object));
    }

    media_session::~media_session() {
        ASSERT(shutdown());
    }

    bool media_session::begin_get_event(async_callback* callback) const {
        return SUCCEEDED(com_object->BeginGetEvent(callback, nullptr));
    }

    media_event media_session::end_get_event(IMFAsyncResult* result) const {
        com_ptr<IMFMediaEvent> event;
        HRESULT hr = com_object->EndGetEvent(result, &event);
        return media_event { SUCCEEDED(hr) ? event : nullptr };
    }

    bool media_session::set_topology(const media_source& src, HWND hwnd, DWORD flags) const {
        return set_topology(topology { src.create_presentation_descriptor(), src, hwnd }, flags);
    }

    bool media_session::set_topology(const presentation_descriptor& pd, const media_source& src, HWND hwnd, DWORD flags) const {
        return set_topology(topology { pd, src, hwnd }, flags);
    }

    bool media_session::set_topology(const topology& topology, DWORD flags) const {
        return SUCCEEDED(com_object->SetTopology(flags, topology.object()));
    }

    bool media_session::pause() const {
        return SUCCEEDED(com_object->Pause());
    }

    bool media_session::stop() const {
        return SUCCEEDED(com_object->Stop());
    }

    bool media_session::start() const {
        win32::prop_variant prop;

        return SUCCEEDED(com_object->Start(&GUID_NULL, prop));
    }

    bool media_session::close() const {
        return SUCCEEDED(com_object->Close());
    }

    bool media_session::shutdown() const {
        return SUCCEEDED(com_object->Shutdown());
    }

    bool media_session::get_service(const GUID& service, const IID& riid, LPVOID* object) const {
        return SUCCEEDED(MFGetService(com_object, service, riid, object));
    }

    display_control media_session::display() const {
        com_ptr<IMFVideoDisplayControl> ctrl;
        bool success = get_service(MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&ctrl));

        return display_control { success ? ctrl : nullptr };
    }



    int64_t presentation_descriptor::descriptor_count() const {
        DWORD count;
        HRESULT hr = com_object->GetStreamDescriptorCount(&count);
        return SUCCEEDED(hr) ? count : -1;
    }

    bool presentation_descriptor::descriptor(int64_t index, bool& selected, IMFStreamDescriptor** sd) const {
        BOOL s;
        HRESULT hr = com_object->GetStreamDescriptorByIndex(utils::narrow<DWORD>(index), &s, sd);
        selected = s;

        return SUCCEEDED(hr);
    }

    media_source::media_source(const istream_ptr& stream, const std::string& path)
        : media_source(source_resolver(), stream, path) { }

    media_source::media_source(const source_resolver& resolver, const istream_ptr& stream, const std::string& path)
        : _stream { std::make_unique<binary_stream_imfbytestream>(stream) } {
        com_ptr<IUnknown> obj = resolver.create_source(*_stream, path);
        ASSERT(obj);
        HASSERT(obj->QueryInterface(&com_object));
    }

    media_source::~media_source() {
        ASSERT(shutdown());
    }

    presentation_descriptor media_source::create_presentation_descriptor() const {
        com_ptr<IMFPresentationDescriptor> pd;
        HRESULT hr = com_object->CreatePresentationDescriptor(&pd);

        return presentation_descriptor { SUCCEEDED(hr) ? pd : nullptr };
    }

    bool media_source::shutdown() const {
        return SUCCEEDED(com_object->Shutdown());
    }

    source_resolver::source_resolver() {
        HASSERT(MFCreateSourceResolver(&com_object));
    }

    com_ptr<IUnknown> source_resolver::create_source(binary_stream_imfbytestream& bs, const std::string& path) const {
        com_ptr<IUnknown> unk;
        MF_OBJECT_TYPE type;
        HRESULT hr = com_object->CreateObjectFromByteStream(&bs, utils::utf16(path).c_str(),
            MF_RESOLUTION_KEEP_BYTE_STREAM_ALIVE_ON_FAIL | MF_RESOLUTION_MEDIASOURCE,
            nullptr, &type, &unk);

        return (SUCCEEDED(hr) && type == MF_OBJECT_MEDIASOURCE) ? unk : nullptr;
    }

    topology::topology() {
        HASSERT(MFCreateTopology(&com_object));
    }

    topology::topology(const presentation_descriptor& pd, const media_source& src, HWND hwnd) : topology() {
        int64_t count = pd.descriptor_count();
        ASSERT(count > 0);

        for (int64_t i = 0; i < count; ++i) {
            _add_branch(pd, src, hwnd, utils::narrow<DWORD>(i));
        }
    }

    void topology::_add_branch(const presentation_descriptor& pd, const media_source& src, HWND hwnd, DWORD index) {
        com_ptr<IMFStreamDescriptor> sd;
        bool selected;
        ASSERT(pd.descriptor(index, selected, &sd));
        if (selected) {
            com_ptr<IMFActivate> activate;
            _create_sink_activate(sd, hwnd, &activate);

            com_ptr<IMFTopologyNode> source;
            _add_source_node(pd, src, sd, &source);
            com_ptr<IMFTopologyNode> output;
            _add_output_node(activate, 0, &output);

            HASSERT(source->ConnectOutput(0, output, 0));
        }
    }

    void topology::_create_sink_activate(IMFStreamDescriptor* sd, HWND hwnd, IMFActivate** activate) {
        (void) this;
        com_ptr<IMFMediaTypeHandler> handler;
        HASSERT(sd->GetMediaTypeHandler(&handler));

        GUID major;
        HASSERT(handler->GetMajorType(&major));

        com_ptr<IMFActivate> act;
        if (major == MFMediaType_Audio) {
            HASSERT(MFCreateAudioRendererActivate(&act));
        } else if (major == MFMediaType_Video) {
            HASSERT(MFCreateVideoRendererActivate(hwnd, &act));
        } else {
            ASSERT(false);
        }

        act->AddRef();
        *activate = act.GetInterfacePtr();
    }

    void topology::_add_source_node(const presentation_descriptor& pd, const media_source& src, IMFStreamDescriptor* sd, IMFTopologyNode** node) {
        (void) this;
        com_ptr<IMFTopologyNode> _node;
        HASSERT(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &_node));
        HASSERT(_node->SetUnknown(MF_TOPONODE_SOURCE, src.object()));
        HASSERT(_node->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pd.object()));
        HASSERT(_node->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, sd));
        HASSERT(com_object->AddNode(_node));

        _node->AddRef();
        *node = _node.GetInterfacePtr();

    }

    void topology::_add_output_node(IMFActivate* activate, DWORD id, IMFTopologyNode** node) {
        (void) this;
        com_ptr<IMFTopologyNode> _node;
        HASSERT(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &_node));
        HASSERT(_node->SetObject(activate));
        HASSERT(_node->SetUINT32(MF_TOPONODE_STREAMID, id));
        HASSERT(_node->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE));
        HASSERT(com_object->AddNode(_node));
        _node->AddRef();
        *node = _node.GetInterfacePtr();
    }



    HRESULT async_callback::QueryInterface(const IID& riid, void** ppvObject) {
        static const QITAB qit[] {
            QITABENT(async_callback, IMFAsyncCallback), { }
        };
        return QISearch(this, qit, riid, ppvObject);
    }

    ULONG async_callback::AddRef() {
        return InterlockedIncrement(&_refcount);
    }

    ULONG async_callback::Release() {
        uint32_t count = InterlockedDecrement(&_refcount);

        if (count == 0) {
            delete this;
        }

        return count;
    }

    HRESULT async_callback::GetParameters(DWORD*, DWORD*) {
        return E_NOTIMPL;
    }

    HRESULT async_callback::Invoke(IMFAsyncResult* pAsyncResult) {
        return invoke(pAsyncResult) ? S_OK : E_FAIL;
    }

}
