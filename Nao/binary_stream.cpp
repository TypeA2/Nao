#include "binary_stream.h"

#include "utils.h"

#include <fstream>

#include <mfapi.h>
#include <Shlwapi.h>

#pragma region IMF interfaces

ULONG binary_istream::AddRef() {
    InterlockedIncrement(&_m_refcount);
    return _m_refcount;
}

ULONG binary_istream::Release() {
    uint32_t refcount = InterlockedDecrement(&_m_refcount);

    if (_m_refcount == 0) {
        delete this;
    }

    return refcount;
}

HRESULT binary_istream::QueryInterface(const IID& riid, void** ppvObject) {
    static const QITAB qit[] = {
        QITABENT(binary_istream, IMFAsyncCallback),
        QITABENT(binary_istream, IMFByteStream),
        { }
    };

    return QISearch(this, qit, riid, ppvObject);
}

HRESULT binary_istream::GetCapabilities(DWORD* pdwCapabilities) {
    if (!pdwCapabilities) {
        return E_INVALIDARG;
    }

    *pdwCapabilities = MFBYTESTREAM_IS_READABLE | MFBYTESTREAM_IS_SEEKABLE | MFBYTESTREAM_DOES_NOT_USE_NETWORK;

    return S_OK;
}

HRESULT binary_istream::GetLength(QWORD* pqwLength) {
    if (!good()) {
        return E_ABORT;
    }

    if (!pqwLength) {
        return E_INVALIDARG;
    }

    auto current = tellg();
    seekg(0, std::ios::end);
    *pqwLength = tellg();
    seekg(current);

    return S_OK;
}

HRESULT binary_istream::SetLength(QWORD qwLength) {
    return E_NOTIMPL;
}

HRESULT binary_istream::GetCurrentPosition(QWORD* pqwPosition) {
    if (!good()) {
        return E_ABORT;
    }

    if (!pqwPosition) {
        return E_INVALIDARG;
    }

    *pqwPosition = tellg();

    return S_OK;
}

HRESULT binary_istream::SetCurrentPosition(QWORD qwPosition) {
    if (!good()) {
        return E_ABORT;
    }

    seekg(qwPosition);

    return good() ? S_OK : E_ABORT;
}

HRESULT binary_istream::IsEndOfStream(BOOL* pfEndOfStream) {
    if (!good()) {
        return E_ABORT;
    }

    if (!pfEndOfStream) {
        return E_INVALIDARG;
    }

    *pfEndOfStream = eof();

    return S_OK;
}

HRESULT binary_istream::Read(BYTE* pb, ULONG cb, ULONG* pcbRead) {
    if (!good()) {
        return E_ABORT;
    }

    if (!pb || cb == 0 || !pcbRead) {
        return E_INVALIDARG;
    }

    auto start = tellg();
    read(pb, cb);

    *pcbRead = ULONG(tellg() - start);

    return (*pcbRead == cb) ? S_OK : E_FAIL;
}

HRESULT binary_istream::BeginRead(BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) {
    if (!good()) {
        return E_ABORT;
    }

    if (!pb || cb == 0 || !pCallback) {
        return E_INVALIDARG;
    }

    IMFAsyncResult* result = nullptr;
    auto obj = new async_result(pb, cb);
    ASSERT(SUCCEEDED(MFCreateAsyncResult(obj, pCallback, punkState, &result)));
    
    ASSERT(SUCCEEDED(MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, this, result)));
    result->Release();

    return S_OK;
}

HRESULT binary_istream::EndRead(IMFAsyncResult* pResult, ULONG* pcbRead) {
    if (!pResult || !pcbRead) {
        return E_INVALIDARG;
    }

    if (HRESULT hr = pResult->GetStatus();  FAILED(hr)) {
        return hr;
    }

    IUnknown* unk = nullptr;
    ASSERT(SUCCEEDED(pResult->GetObjectW(&unk)));

    async_result* res = dynamic_cast<async_result*>(unk);

    *pcbRead = res->read;

    unk->Release();

    return S_OK;

}

HRESULT binary_istream::Write(const BYTE* pb, ULONG cb, ULONG* pcbWritten) {
    return E_NOTIMPL;
}

HRESULT binary_istream::BeginWrite(const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) {
    return E_NOTIMPL;
}

HRESULT binary_istream::EndWrite(IMFAsyncResult* pResult, ULONG* pcbWritten) {
    return E_NOTIMPL;
}

HRESULT binary_istream::Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, DWORD dwSeekFlags, QWORD* pqwCurrentPosition) {
    if (!good()) {
        return E_ABORT;
    }

    if (dwSeekFlags & MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO) {
        return E_NOTIMPL;
    }

    if (!pqwCurrentPosition) {
        return E_INVALIDARG;
    }

    switch (SeekOrigin) {
        case msoBegin:
            seekg(llSeekOffset);
            break;

        case msoCurrent:
            rseek(llSeekOffset);
            break;
    }

    *pqwCurrentPosition = tellg();

    return S_OK;
}

HRESULT binary_istream::Flush() {
    // Read-only, so no-op
    return S_OK;
}

HRESULT binary_istream::Close() {
    // Closing is handled elsewhere
    return S_OK;
}

HRESULT binary_istream::GetParameters(DWORD* pdwFlags, DWORD* pdwQueue) {
    return E_NOTIMPL;
}

HRESULT binary_istream::Invoke(IMFAsyncResult* pAsyncResult) {
    if (!good()) {
        return E_ABORT;
    }

    if (!pAsyncResult) {
        return E_INVALIDARG;
    }

    IUnknown* unk_state = nullptr;
    ASSERT(SUCCEEDED(pAsyncResult->GetState(&unk_state)));

    IMFAsyncResult* operation_result = nullptr;
    ASSERT(SUCCEEDED(unk_state->QueryInterface(&operation_result)));

    IUnknown* unk_result = nullptr;
    ASSERT(SUCCEEDED(operation_result->GetObjectW(&unk_result)));

    async_result* result = dynamic_cast<async_result*>(unk_result);

    auto start = tellg();
    read(result->buf, result->count);
    result->read = ULONG(tellg() - start);

    if (operation_result) {
        operation_result->SetStatus(good() ? S_OK : E_ABORT);
        MFInvokeCallback(operation_result);
    }

    unk_state->Release();
    unk_result->Release();
    operation_result->Release();

    return S_OK;
}

binary_istream::async_result::async_result(BYTE* buf, ULONG count)
    : buf { buf }
    , count { count }
    , _m_refcount { 0 }{
    async_result::AddRef();
}


ULONG binary_istream::async_result::AddRef() {
    InterlockedIncrement(&_m_refcount);
    return _m_refcount;
}

ULONG binary_istream::async_result::Release() {
    uint32_t refcount = InterlockedDecrement(&_m_refcount);

    if (_m_refcount == 0) {
        delete this;
    }

    return refcount;
}

HRESULT binary_istream::async_result::QueryInterface(const IID& riid, void** ppvObject) {
    static const QITAB qit[] = {
        QITABENT(async_result, IUnknown),
        QITABENT(async_result, async_result),
        { }
    };

    return QISearch(this, qit, riid, ppvObject);
}

#pragma endregion

binary_istream::binary_istream(const std::string& path)
    : _m_refcount { 1 }
    , file { std::make_unique<std::fstream>(path, std::ios::in | std::ios::binary) } {
    
}

binary_istream::binary_istream(const std::filesystem::path& path)
    : _m_refcount { 1 }
    , file { std::make_unique<std::fstream>(path, std::ios::in | std::ios::binary) } {
    
}


binary_istream::binary_istream(binary_istream&& other) noexcept
    : _m_refcount { other._m_refcount }
    , file { std::move(other.file) }  {
    other.file = nullptr;
}

std::streampos binary_istream::tellg() const {
    std::unique_lock lock(mutex);

    return file->tellg();
}

class binary_istream& binary_istream::seekg(pos_type pos) {
    std::unique_lock lock(mutex);
    file->seekg(pos);

    return *this;
}

binary_istream& binary_istream::seekg(pos_type pos, seekdir dir) {
    std::unique_lock lock(mutex);
    file->seekg(pos, dir);

    return *this;
}

bool binary_istream::eof() const {
    std::unique_lock lock(mutex);
    return file->eof();
}

binary_istream& binary_istream::ignore(std::streamsize max, std::istream::int_type delim) {
    std::unique_lock lock(mutex);
    file->ignore(max, delim);

    return *this;
}

bool binary_istream::good() const {
    std::unique_lock lock(mutex);
    return file->good();
}

class binary_istream& binary_istream::rseek(pos_type pos) {
    std::unique_lock lock(mutex);
    file->seekg(pos, std::ios::cur);

    return *this;
}

binary_istream& binary_istream::read(char* buf, std::streamsize count) {
    std::unique_lock lock(mutex);
    file->read(buf, count);

    return *this;
}
