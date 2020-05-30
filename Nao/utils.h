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

#define CHECK(cond) if (!(cond)) { nao::coutln("Error in:", #cond); DebugBreak(); return false; }
#endif

#define HASSERT(hr) ASSERT(SUCCEEDED(hr))

namespace utils {
    inline namespace arithmetic {
        static constexpr uint64_t make_quad(uint32_t low, uint32_t high) {
            return (static_cast<uint64_t>(high) << 32) | low;
        }

        template <std::integral Out, concepts::arithmetic In >
        static constexpr Out narrow(In val)
        requires (std::numeric_limits<Out>::digits < std::numeric_limits<In>::digits) {
            if constexpr (std::signed_integral<Out> && std::unsigned_integral<In>) {
                // Clamp with 0
                return static_cast<Out>(std::clamp<In>(val, 0, std::numeric_limits<Out>::max()));
            } else {
                return static_cast<Out>(std::clamp<In>(val, std::numeric_limits<Out>::min(), std::numeric_limits<Out>::max()));
            }
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

    static coordinates from_lparam(LPARAM lparam);
};

struct rectangle {
    int64_t x;
    int64_t y;
    int64_t width;
    int64_t height;
};

struct dimensions {
    int64_t width;
    int64_t height;

    rectangle rect() const;

    static dimensions from_lparam(LPARAM lparam);
};
