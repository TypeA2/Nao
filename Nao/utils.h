#pragma once

#include <sstream>
#include <chrono>
#include <algorithm>
#include <limits>

#include "frameworks.h"
#include "concepts.h"

#define LINE_STRINGIFY(x) LINE_STRINGIFY2(x)
#define LINE_STRINGIFY2(x) #x
#define FILE_AT __FILE__ ":" LINE_STRINGIFY(__LINE__)

#ifdef NDEBUG
#define ASSERT(cond) do { if (!(cond)) { \
    constexpr const char* str = "assertion failed in " FILE_AT " -> " #cond; \
    utils::coutln(str); \
    throw std::runtime_error(str); }} while (0)

#define CHECK(cond) if (!(cond)) { utils::coutln("Error in:", #cond); return false; }

#else
#include <cassert>
#define ASSERT(cond) assert(cond);

#define CHECK(cond) if (!(cond)) { utils::coutln("Error in:", #cond); DebugBreak(); return false; }
#endif

#define HASSERT(hr) ASSERT(SUCCEEDED(hr))

namespace utils {
    inline namespace logging {
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
    }

    inline namespace formatting {
        std::string bytes(int64_t n);
        std::wstring wbytes(int64_t n);

        std::string bits(int64_t n);

        std::string perc(double p);
        std::wstring wperc(double p);

        std::string utf8(const std::wstring& str);
        std::wstring utf16(const std::string& str);

        std::string format_hours(std::chrono::nanoseconds ns, bool ms = true);
        std::string format_minutes(std::chrono::nanoseconds ns, bool ms = true);
    }

    inline namespace arithmetic {
        static constexpr uint64_t make_quad(uint32_t low, uint32_t high) {
            return (static_cast<uint64_t>(high) << 32) | low;
        }

        template <std::integral Out, concepts::arithmetic In >
        static constexpr Out narrow(In val) requires (std::numeric_limits<Out>::digits < std::numeric_limits<In>::digits) {
            return static_cast<Out>(std::clamp<In>(val, std::numeric_limits<Out>::min(), std::numeric_limits<Out>::max()));
        }

        template <concepts::floating_point Out, concepts::arithmetic In>
        static constexpr Out narrow(In val) requires (std::numeric_limits<Out>::digits < std::numeric_limits<In>::digits) {
            return static_cast<Out>(std::clamp<In>(val, -1i64 * (1i64 << std::numeric_limits<Out>::digits), (1i64 << std::numeric_limits<Out>::digits) - 1));
        }
    }
}

struct coordinates {
    int64_t x;
    int64_t y;
};

struct dimensions {
    int64_t width;
    int64_t height;
};

struct rectangle {
    int64_t x;
    int64_t y;
    int64_t width;
    int64_t height;
};
