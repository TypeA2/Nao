#pragma once

#include <istream>
#include <mutex>
#include <filesystem>

#include <mfidl.h>

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
    struct DECLSPEC_UUID("1E89C9C0-318E-4E1B-9EE1-81A586111507") async_result : IUnknown {
        explicit async_result(BYTE* buf, ULONG count);
        virtual ~async_result() = default;

        STDMETHODIMP_(ULONG) AddRef() override;
        STDMETHODIMP_(ULONG) Release() override;
        STDMETHODIMP QueryInterface(const IID& riid, void** ppvObject) override;

        BYTE* buf;
        ULONG count;
        ULONG read;
        private:
        volatile uint32_t _m_refcount;
    };
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

    template <typename T>
    std::enable_if_t<std::is_arithmetic_v<T>, binary_istream&>
        read(T* buf, std::streamsize count) {
        return read(reinterpret_cast<char*>(buf), count);
    }

    template <typename Container>
    binary_istream& read(Container& buf) {
        return read(buf.data(), buf.size() * sizeof(Container::value_type));
    }

    template <typename T, size_t size>
    std::enable_if_t<std::is_arithmetic_v<T>, binary_istream&>
        read(T (& buf)[size]) {
        return read(buf, size);
    }

    template <typename T, std::endian endian = std::endian::native>
    std::enable_if_t<std::is_arithmetic_v<T>, T>
        read() {
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

    protected:
    std::unique_ptr<std::istream> file;
    mutable std::mutex mutex;
};

