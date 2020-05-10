#include "wic.h"

#include <typeinfo>

static const com_ptr<IWICImagingFactory>& static_factory() {
    static com_ptr<IWICImagingFactory> factory;
    if (!factory) {
        HASSERT(factory.CreateInstance(CLSID_WICImagingFactory));
    }

    return factory;
}

namespace wic {
    binary_stream_istream::binary_stream_istream(istream_ptr stream) : _stream { std::move(stream) } {

    }

    HRESULT binary_stream_istream::QueryInterface(const IID& riid, void** ppvObject) {
        static QITAB tab[] {
            QITABENT(binary_stream_istream, IStream),
            QITABENT(binary_stream_istream, ISequentialStream),
            {  }
        };

        return QISearch(this, tab, riid, ppvObject);
    }

    ULONG binary_stream_istream::AddRef() {
        InterlockedIncrement(&_refcount);
        return _refcount;
    }

    ULONG binary_stream_istream::Release() {
        auto ref = InterlockedDecrement(&_refcount);
        if (ref == 0) {
            delete this;
        }

        return ref;
    }

    HRESULT binary_stream_istream::Clone(IStream** ppstm) {
        *ppstm = nullptr;
        return E_NOTIMPL;
    }

    HRESULT binary_stream_istream::Commit(DWORD) {
        return E_NOTIMPL;
    }

    HRESULT binary_stream_istream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) {
        size_t read = 0;
        size_t written = 0;
        char buf[4096];

        HRESULT res = S_OK;
        while (read < cb.QuadPart) {
            if (_stream->eof()) {
                res = E_FAIL;
                break;
            }

            size_t to_read = std::min<size_t>(sizeof(buf), cb.QuadPart - read);
            _stream->read(buf, to_read);

            ULONG written_now;
            pstm->Write(buf, static_cast<ULONG>(_stream->gcount()), &written_now);
            read += _stream->gcount();
            written += written_now;
        }

        if (pcbRead) {
            pcbRead->QuadPart = read;
        }

        if (pcbWritten) {
            pcbWritten->QuadPart = written;
        }

        return res;
    }

    HRESULT binary_stream_istream::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
        return E_NOTIMPL;
    }

    HRESULT binary_stream_istream::Revert() {
        return S_OK;
    }

    HRESULT binary_stream_istream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) {
        switch (dwOrigin) {
            case STREAM_SEEK_SET: _stream->seekg(dlibMove.QuadPart); break;
            case STREAM_SEEK_CUR: _stream->seekg(dlibMove.QuadPart, std::ios::cur); break;
            case STREAM_SEEK_END: _stream->seekg(dlibMove.QuadPart, std::ios::end); break;
            default: break;
        }

        if (plibNewPosition) {
            plibNewPosition->QuadPart = _stream->tellg();
        }

        return S_OK;
    }

    HRESULT binary_stream_istream::SetSize(ULARGE_INTEGER) {
        return E_NOTIMPL;
    }

    HRESULT binary_stream_istream::Stat(STATSTG* pstatstg, DWORD) {
        if (!pstatstg) {
            return E_POINTER;
        }

        pstatstg->pwcsName = nullptr;

        pstatstg->type = STGTY_STREAM;

        auto cur = _stream->tellg();
        _stream->seekg(0, std::ios::end);
        pstatstg->cbSize.QuadPart = _stream->tellg();
        _stream->seekg(cur);

        pstatstg->grfMode = GENERIC_READ;

        return S_OK;
    }

    HRESULT binary_stream_istream::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
        return E_NOTIMPL;
    }

    HRESULT binary_stream_istream::Read(void* pv, ULONG cb, ULONG* pcbRead) {
        _stream->read(static_cast<char*>(pv), cb);
        if (pcbRead) {
            *pcbRead = static_cast<ULONG>(_stream->gcount());
        }

        return S_OK;
    }

    HRESULT binary_stream_istream::Write(const void*, ULONG, ULONG*) {
        return E_NOTIMPL;
    }



    WICPixelFormatGUID bitmap_source::pixel_format() const {
        WICPixelFormatGUID format;
        HRESULT hr = com_object->GetPixelFormat(&format);

        return SUCCEEDED(hr) ? format : GUID_NULL;
    }

    dimensions bitmap_source::size() const {
        UINT width;
        UINT height;
        HRESULT hr = com_object->GetSize(&width, &height);

        return SUCCEEDED(hr) ? dimensions { width, height } : dimensions { };
    }

    std::vector<char> bitmap_source::get_pixels() const {
        if (pixel_format() != GUID_WICPixelFormat32bppPBGRA) {
            return { };
        }

        auto [w, h] = size();
        std::vector<char> data(4ui64 * w * h);
        HRESULT hr = com_object->CopyPixels(nullptr, utils::narrow<UINT>(4ui64 * w),
            utils::narrow<UINT>(data.size()), reinterpret_cast<BYTE*>(data.data()));

        return SUCCEEDED(hr) ? data : std::vector<char>{ };
    }



    bitmap_decoder::bitmap_decoder(const istream_ptr& stream) : bitmap_decoder(imaging_factory { }, stream) { }

    bitmap_decoder::bitmap_decoder(const imaging_factory& factory, const istream_ptr& stream)
        : _stream { stream } {
        com_object = factory.create_decoder(_stream);
        ASSERT(com_object);
    }

    bitmap_frame_decode bitmap_decoder::get_frame(int64_t index) const {
        com_ptr<IWICBitmapFrameDecode> frame;
        HRESULT hr = com_object->GetFrame(utils::narrow<UINT>(index), &frame);

        return bitmap_frame_decode(SUCCEEDED(hr) ? frame : nullptr);
    }



    format_converter::format_converter() : format_converter(imaging_factory { }) { }
    format_converter::format_converter(const imaging_factory& factory)
        : bitmap_source { factory.create_converter() }, _converter { com_object } { }

    bool format_converter::can_convert(const WICPixelFormatGUID& from, const WICPixelFormatGUID& to) const {
        BOOL can_convert;
        HRESULT hr = _converter->CanConvert(from, to, &can_convert);

        return SUCCEEDED(hr) ? can_convert : false;
    }
    
    bool format_converter::initialize(bitmap_source& source, const WICPixelFormatGUID& format) const {
        HRESULT hr = _converter->Initialize(source.object(), format, WICBitmapDitherTypeNone,
            nullptr, 0.f, WICBitmapPaletteTypeCustom);

        return SUCCEEDED(hr);
    }



    imaging_factory::imaging_factory() : imaging_factory { static_factory() } { }
    imaging_factory::imaging_factory(const com_ptr<IWICImagingFactory>& ptr) : com_interface { ptr } { }

    com_ptr<IWICBitmapDecoder> imaging_factory::create_decoder(binary_stream_istream& stream) const {
        IWICBitmapDecoder* decoder;
        HRESULT hr = com_object->CreateDecoderFromStream(&stream, nullptr, WICDecodeMetadataCacheOnDemand, &decoder);

        return SUCCEEDED(hr) ? decoder : nullptr;
    }

    com_ptr<IWICFormatConverter> imaging_factory::create_converter() const {
        com_ptr<IWICFormatConverter> converter;
        HRESULT hr = com_object->CreateFormatConverter(&converter);

        return SUCCEEDED(hr) ? converter : nullptr;
    }

}