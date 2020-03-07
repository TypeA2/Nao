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



    media_session::media_session() {
        HASSERT(MFStartup(MF_VERSION));
        HASSERT(MFCreateMediaSession(nullptr, &com_object));
    }

    media_session::~media_session() {
        HASSERT(MFShutdown());
    }

    bool media_session::begin_get_event(IMFAsyncCallback* callback) const {
        return SUCCEEDED(com_object->BeginGetEvent(callback, nullptr));
    }

    bool media_session::end_get_event(IMFAsyncResult* result, IMFMediaEvent** event) const {
        return SUCCEEDED(com_object->EndGetEvent(result, event));
    }

    bool media_session::set_topology(IMFTopology* topology, DWORD flags) const {
        return SUCCEEDED(com_object->SetTopology(flags, topology));
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


}
