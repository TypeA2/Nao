#pragma once

#include "utils.h"
#include "binary_stream.h"

#include "com.h"

#include <mfapi.h>
#include <mfidl.h>
#include <Mferror.h>
#include <evr.h>

namespace mf {
    class binary_stream_imfbytestream : public IMFByteStream, IMFAsyncCallback {
        istream_ptr _stream;
        volatile uint32_t _refcount = 1;
        public:
        binary_stream_imfbytestream(const istream_ptr& stream) : _stream { stream } { }
        virtual ~binary_stream_imfbytestream() = default;

        // IUnknown
        STDMETHODIMP QueryInterface(const IID& riid, void** ppvObject) override;
        STDMETHODIMP_(ULONG) AddRef() override;
        STDMETHODIMP_(ULONG) Release() override;

        // IMFByteStream
        STDMETHODIMP BeginRead(BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) override;
        STDMETHODIMP EndRead(IMFAsyncResult* pResult, ULONG* pcbRead) override;
        STDMETHODIMP Read(BYTE* pb, ULONG cb, ULONG* pcbRead) override;
        STDMETHODIMP GetCapabilities(DWORD* pdwCapabilities) override;
        STDMETHODIMP GetCurrentPosition(QWORD* pqwPosition) override;
        STDMETHODIMP GetLength(QWORD* pqwLength) override;
        STDMETHODIMP IsEndOfStream(BOOL* pfEndOfStream) override;
        STDMETHODIMP Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, DWORD, QWORD* pqwCurrentPosition) override;
        STDMETHODIMP SetCurrentPosition(QWORD qwPosition) override;

        // Not implemented (IMFByteStream)
        STDMETHODIMP BeginWrite(const BYTE*, ULONG, IMFAsyncCallback*, IUnknown*) override;
        STDMETHODIMP EndWrite(IMFAsyncResult*, ULONG*) override;
        STDMETHODIMP Write(const BYTE*, ULONG, ULONG*) override;
        STDMETHODIMP Close() override;
        STDMETHODIMP Flush() override;
        STDMETHODIMP SetLength(QWORD) override;

        // IMFAsyncCallback
        STDMETHODIMP GetParameters(DWORD*, DWORD*) override;
        STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) override;
    };

    class media_session : public com::com_interface<IMFMediaSession> {
        public:
        media_session();
        ~media_session();

        bool begin_get_event(IMFAsyncCallback* callback) const;
        bool end_get_event(IMFAsyncResult* result, IMFMediaEvent** event) const;

        bool set_topology(IMFTopology* topology, DWORD flags = 0) const;

        bool pause() const;
        bool stop() const;
        bool start() const;

        bool close() const;
        bool shutdown() const;

        bool get_service(const GUID& service, const IID& riid, LPVOID* object) const;
    };

    class media_source : public com::com_interface<IMFMediaSource> {
        std::unique_ptr<binary_stream_imfbytestream> _stream;
        public:
        media_source() = default;
        explicit media_source(const istream_ptr& stream, const std::string& path);

        com_ptr<IMFPresentationDescriptor> create_presentation_descriptor() const;


        bool shutdown() const;
    };

    class source_resolver : public com::com_interface<IMFSourceResolver> {
        public:
        source_resolver();

        com_ptr<IUnknown> create_source(binary_stream_imfbytestream& bs, const std::string& path) const;
    };

    class display_control : public com::com_interface<IMFVideoDisplayControl> {
        public:

        void repaint() const;

        bool set_position(const rectangle& rect) const;
    };
}
