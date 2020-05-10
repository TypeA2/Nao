#pragma once

#include "frameworks.h"

#include "binary_stream.h"
#include "com.h"

namespace wic {
    class DECLSPEC_UUID("8D8AE7A1-109B-4491-8CAB-3E059D5680E5") binary_stream_istream : public IStream {
        istream_ptr _stream;
        volatile uint32_t _refcount = 1;

        public:
        explicit binary_stream_istream(istream_ptr stream);
        virtual ~binary_stream_istream() = default;

        STDMETHODIMP QueryInterface(const IID & riid, void** ppvObject) override;
        STDMETHODIMP_(ULONG) AddRef() override;
        STDMETHODIMP_(ULONG) Release() override;

        STDMETHODIMP Clone(IStream * *ppstm) override;
        STDMETHODIMP Commit(DWORD) override;
        STDMETHODIMP CopyTo(IStream * pstm, ULARGE_INTEGER cb, ULARGE_INTEGER * pcbRead, ULARGE_INTEGER * pcbWritten) override;
        STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override;
        STDMETHODIMP Revert() override;
        STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition) override;
        STDMETHODIMP SetSize(ULARGE_INTEGER) override;
        STDMETHODIMP Stat(STATSTG * pstatstg, DWORD) override;
        STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override;

        STDMETHODIMP Read(void* pv, ULONG cb, ULONG * pcbRead) override;
        STDMETHODIMP Write(const void*, ULONG, ULONG*) override;
    };

    class imaging_factory;

    class bitmap_source : public com::com_interface<IWICBitmapSource> {
        public:
        using com_interface::com_interface;

        WICPixelFormatGUID pixel_format() const;

        dimensions size() const;

        std::vector<char> get_pixels() const;
    };

    class bitmap_frame_decode : public bitmap_source {
        public:
        using bitmap_source::bitmap_source;
    };

    class bitmap_decoder : public com::com_interface<IWICBitmapDecoder >{
        binary_stream_istream _stream;

        public:
        bitmap_decoder() = delete;
        explicit bitmap_decoder(const istream_ptr& stream);
        bitmap_decoder(const imaging_factory& factory, const istream_ptr& stream);

        bitmap_frame_decode get_frame(int64_t index) const;
    };

    class format_converter : public bitmap_source {
        com_ptr<IWICFormatConverter> _converter;

        public:
        explicit format_converter();
        explicit format_converter(const imaging_factory& factory);

        bool can_convert(const WICPixelFormatGUID& from, const WICPixelFormatGUID& to) const;
        bool initialize(bitmap_source& source, const WICPixelFormatGUID& format) const;
    };

    class imaging_factory : public com::com_interface<IWICImagingFactory> {
        public:
        explicit imaging_factory();
        explicit imaging_factory(const com_ptr<IWICImagingFactory>& ptr);

        com_ptr<IWICBitmapDecoder> create_decoder(binary_stream_istream& stream) const;
        com_ptr<IWICFormatConverter> create_converter() const;
    };
}
