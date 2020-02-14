#pragma once

#include <sstream>
#include <chrono>

#define LINE_STRINGIFY(x) LINE_STRINGIFY2(x)
#define LINE_STRINGIFY2(x) #x
#define FILE_AT __FILE__ ":" LINE_STRINGIFY(__LINE__)

#ifdef NDEBUG
#define ASSERT(cond) do { if (!(cond)) { \
    constexpr const char* str = "assertion failed in " FILE_AT " -> " #cond; \
    utils::coutln(str); \
    throw std::runtime_error(str); }} while (0)

#else
#include <cassert>
#define ASSERT(cond) assert(cond);
#endif

#define HASSERT(hr) ASSERT(SUCCEEDED(hr))

namespace utils {
    void cout(const char* str);
    void cout(const wchar_t* wstr);

    template <typename T>
    void cout(std::basic_string<T, std::char_traits<T>, std::allocator<T>> str) {
        std::basic_stringstream<T, std::char_traits<T>, std::allocator<T>> ss;
        ss << str;
        cout(ss.str().c_str());
    }

    template <typename T>
    void cout(T&& v) {
        std::wstringstream ss;
        ss << v;
        cout(ss.str().c_str());
    }

    template <typename T>
    void coutln(T&& v) {
        cout(std::forward<T>(v));
        cout("\n");
    }

    template <typename T, typename... Args>
    void cout(T&& v, Args&&... args) {
        cout(std::forward<T>(v));
        cout(" ");

        if constexpr (sizeof...(Args) > 0) {
            cout(std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    void coutln(Args&&... args) {
        cout(std::forward<Args>(args)...);
        cout("\n");
    }

    std::string bytes(int64_t n);
    std::wstring wbytes(int64_t n);

    std::string bits(int64_t n);

    std::string perc(double p);
    std::wstring wperc(double p);

    std::string utf8(const std::wstring& str);
    std::wstring utf16(const std::string& str);

    static constexpr uint64_t make_quad(uint32_t low, uint32_t high) {
        return (static_cast<uint64_t>(high) << 32) | low;
    }

    std::string format_hours(std::chrono::nanoseconds ns, bool ms = true);
    std::string format_minutes(std::chrono::nanoseconds ns, bool ms = true);
}
