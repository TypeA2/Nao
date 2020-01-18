#pragma once

#include "frameworks.h"

#include <sstream>

#ifdef NDEBUG
#define ASSERT(cond) do { if (!(cond)) { throw std::runtime_error("assertion failed in " __FILE__ " at " __LINE__ ": " #cond); }} while (0)
#else
#include <cassert>
#define ASSERT(cond) assert(cond);
#endif

namespace utils {
    void cout(LPCSTR str);
    void cout(LPCWSTR wstr);

    void coutln(LPCSTR str);
    void coutln(LPCWSTR wstr);

    template <typename T>
    void cout(T&& v) {
        std::wstringstream ss;
        ss << v;
        cout(ss.str().c_str());
    }

    template <typename T>
    void cout(const std::basic_string<T>& str) {
        std::basic_stringstream<T> ss;
        ss << str;
        cout(ss.str().c_str());
    }

    template <typename T>
    void coutln(const T& v) {
        cout(v);
        cout("\n");
    }

    template <typename T, typename... Args>
    void cout(T&& v, Args&&... args) {
        cout(std::forward<T>(v));
        cout(" ");
        cout(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void coutln(Args&&... args) {
        cout(std::forward<Args>(args)...);
        cout("\n");
    }

    std::string bytes(int64_t n);
    std::wstring wbytes(int64_t n);

    std::string perc(double p);
    std::wstring wperc(double p);

    std::string utf8(const std::wstring& str);
    std::wstring utf16(const std::string& str);
}
