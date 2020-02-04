#pragma once

#include <istream>
#include <mutex>
#include <filesystem>

#include "frameworks.h"
#include "concepts.h"

class binary_istream : IMFAsyncCallback, public IMFByteStream {
#pragma region IMF interfaces
    public:
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    STDMETHODIMP QueryInterface(const IID& riid, void** ppvObject) override;

    STDMETHODIMP GetCapabilities(DWORD* pdwCapabilities) override;
    STDMETHODIMP GetLength(QWORD* pqwLength) override;
    STDMETHODIMP SetLength(QWORD qwLength) override;
    STDMETHODIMP GetCurrentPosition(QWORD* pqwPosition) override;
    STDMETHODIMP SetCurrentPosition(QWORD qwPosition) override;
    STDMETHODIMP IsEndOfStream(BOOL* pfEndOfStream) override;
    STDMETHODIMP Read(BYTE* pb, ULONG cb, ULONG* pcbRead) override;
    STDMETHODIMP BeginRead(BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) override;
    STDMETHODIMP EndRead(IMFAsyncResult* pResult, ULONG* pcbRead) override;
    STDMETHODIMP Write(const BYTE* pb, ULONG cb, ULONG* pcbWritten) override;
    STDMETHODIMP BeginWrite(const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) override;
    STDMETHODIMP EndWrite(IMFAsyncResult* pResult, ULONG* pcbWritten) override;
    STDMETHODIMP Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, DWORD dwSeekFlags, QWORD* pqwCurrentPosition) override;
    STDMETHODIMP Flush() override;
    STDMETHODIMP Close() override;

    STDMETHODIMP GetParameters(DWORD* pdwFlags, DWORD* pdwQueue) override;
    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) override;

    private:
    volatile uint32_t _m_refcount;

#pragma endregion

    public:
    using pos_type = std::istream::pos_type;

    using seekdir = std::istream::seekdir;

    explicit binary_istream(const std::string& path);
    explicit binary_istream(const std::filesystem::path& path);

    explicit binary_istream(binary_istream&& other) noexcept;
    explicit binary_istream(const binary_istream& other) = delete;

    virtual ~binary_istream() = default;

    virtual std::streampos tellg() const;
    virtual binary_istream& seekg(pos_type pos);
    virtual binary_istream& seekg(pos_type pos, seekdir dir);
    virtual bool eof() const;
    virtual binary_istream& ignore(std::streamsize max,
        std::istream::int_type delim = std::istream::traits_type::eof());

    virtual bool good() const;

    // Seek with std::ios::cur
    binary_istream& rseek(pos_type pos);

    virtual binary_istream& read(char* buf, std::streamsize count);

    // Read an arithmetic (integer or floating-point) value array
    template <concepts::arithmetic T>
    binary_istream& read(T* buf, std::streamsize count) {
        return read(reinterpret_cast<char*>(buf), count * sizeof(T));
    }

    // Read an arithmetic (integer or floating-point) C value array
    template <concepts::arithmetic T, size_t size>
    binary_istream& read(T(&buf)[size]) {
        return read(buf, size * sizeof(T));
    }

    // Read any container which exposes ::data(), ::size() and ::value_type
    template <concepts::readable_container Container>
    binary_istream& read(Container& buf) {
        return read(buf.data(), buf.size() * sizeof(Container::value_type));
    }

    // Read an arithmetic value in the specified endianness
    template <concepts::arithmetic T, std::endian endian = std::endian::native>
    T read() {
        T temp {};
        char buf[sizeof(T)];
        read(buf, sizeof(T));

        if constexpr (endian == std::endian::little) {
            for (size_t i = 0; i < sizeof(T); ++i) {
                temp |= (T(uint8_t(buf[i])) << (i * CHAR_BIT));
            }
        } else {
            for (size_t i = (sizeof(T) - 1); i >= 0; --i) {
                temp |= (T(uint8_t(buf[i])) << (i * CHAR_BIT));
            }
        }

        return temp;
    }

    // Read an arithmetic value into the argument
    template <concepts::arithmetic T, std::endian endian = std::endian::native>
    binary_istream& read(T& val) {
        val = read<T>();

        return *this;
    }

    protected:
    std::unique_ptr<std::istream> file;
    mutable std::mutex mutex;
};

