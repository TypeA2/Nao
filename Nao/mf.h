#pragma once

#include "utils.h"
#include "binary_stream.h"

#include "com.h"

#include <mfapi.h>
#include <mfidl.h>
#include <Mferror.h>
#include <evr.h>

class ui_element;
namespace mf {
    inline namespace io {
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
    }
    // MFStartup / MFShutdown RAII guard
    class mf_interface {
        public:
        mf_interface();
        ~mf_interface();
    };

    class topology;
    class media_source;
    class presentation_descriptor;
    class source_resolver;
    class async_callback;

    class display_control : mf_interface, public com::com_interface<IMFVideoDisplayControl> {
        public:
        using com_interface::com_interface;

        void repaint() const;

        bool set_position(const rectangle& rect) const;
    };

    class attributes : mf_interface, public virtual com::com_interface<IMFAttributes> {
        public:
        using com_interface::com_interface;

        uint32_t get_uint32(const GUID& guid) const;
    };

    class media_event : mf_interface, public com::com_interface<IMFMediaEvent> {
        public:
        using com_interface::com_interface;

        MediaEventType type() const;
        HRESULT status() const;
        bool good() const;

        attributes attributes() const;
    };

    class media_session : mf_interface, public com::com_interface<IMFMediaSession> {
        public:
        media_session();
        ~media_session();

        bool begin_get_event(async_callback* callback) const;
        media_event end_get_event(IMFAsyncResult* result) const;

        bool set_topology(const media_source& src, ui_element* window = nullptr, DWORD flags = 0) const;
        bool set_topology(const presentation_descriptor& pd, const media_source& src, ui_element* window = nullptr, DWORD flags = 0) const;
        bool set_topology(const topology& topology, DWORD flags = 0) const;

        bool pause() const;
        bool stop() const;
        bool start() const;

        bool close() const;
        bool shutdown() const;

        bool get_service(const GUID& service, const IID& riid, LPVOID* object) const;

        display_control display() const;
    };

    class presentation_descriptor : mf_interface, public com::com_interface<IMFPresentationDescriptor> {
        public:
        using com_interface::com_interface;

        int64_t descriptor_count() const;
        bool descriptor(int64_t index, bool& selected, IMFStreamDescriptor** sd) const;
    };

    class media_source : mf_interface, public com::com_interface<IMFMediaSource> {
        std::unique_ptr<binary_stream_imfbytestream> _stream;

        public:
        media_source() = default;
        media_source(const istream_ptr& stream, const std::string& path);
        media_source(const source_resolver& resolver, const istream_ptr& stream, const std::string& path);

        ~media_source();

        presentation_descriptor create_presentation_descriptor() const;


        bool shutdown() const;
    };

    class source_resolver : mf_interface, public com::com_interface<IMFSourceResolver> {
        public:
        source_resolver();

        com_ptr<IUnknown> create_source(binary_stream_imfbytestream& bs, const std::string& path) const;
    };

    class topology : mf_interface, public com::com_interface<IMFTopology> {
        public:
        topology();
        explicit topology(const presentation_descriptor& pd, const media_source& src, ui_element* window = nullptr);

        private:
        void _add_branch(const presentation_descriptor& pd, const media_source& src, ui_element* window, DWORD index);

        void _create_sink_activate(IMFStreamDescriptor* sd, ui_element* window, IMFActivate** activate);
        void _add_source_node(const presentation_descriptor& pd, const media_source& src, IMFStreamDescriptor* sd, IMFTopologyNode** node);
        void _add_output_node(IMFActivate* activate, DWORD id, IMFTopologyNode** node);
    };

    class async_callback : IMFAsyncCallback {
        volatile uint32_t _refcount = 1;
        public:
        virtual ~async_callback() = default;

        virtual bool invoke(IMFAsyncResult* result) = 0;

        private:
        STDMETHODIMP QueryInterface(const IID& riid, void** ppvObject) override;
        STDMETHODIMP_(ULONG) AddRef() override;
        STDMETHODIMP_(ULONG) Release() override;
        STDMETHODIMP GetParameters(DWORD*, DWORD*) override;
        STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) override;

        friend class media_session;
    };

    class player : async_callback {
        media_source _source;
        media_session _session;
        display_control _display;

        std::mutex _close_mutex;
        std::condition_variable _close_event;
        bool _can_continue = false;

        ui_element* _msg;

        public:
        player(const istream_ptr& stream, const std::string& path, ui_element* msg, ui_element* display);
        ~player();

        static constexpr UINT WM_APP_PLAYER_EVENT = WM_APP + 1;

        void pause() const;
        void stop() const;
        void start() const;

        bool invoke(IMFAsyncResult* result) override;

        void handle_event(const mf::media_event& event);
        void repaint() const;
        void set_position(const rectangle& rect);
    };
}
