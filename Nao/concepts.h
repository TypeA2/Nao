#pragma once

#include <concepts>
#include <ios>

namespace concepts {

    // Floating point type
    template <typename T>
    concept floating_point = std::is_floating_point_v<T>;

    // Any integral or floating-point type
    template <typename T>
    concept arithmetic = std::integral<T> || std::is_floating_point_v<T>;

    // Any container that exposes value_type, .data and .size
    template <typename Container>
    concept readable_container = arithmetic<typename Container::value_type> &&
    requires (Container & container) {
        { container.data() } -> std::same_as<typename Container::value_type*>;
        { container.size() } -> std::convertible_to<std::streamsize>;
    };

    // If the type is a pointer type
    template <typename P>
    concept pointer = std::is_pointer_v<P>;

    // A pointer or any integer type
    template <typename T>
    concept pointer_or_integral = pointer<T> || std::integral<T>;

    // A smart(-ish) pointer with a getter
    template <typename Container>
    concept smart_pointer = requires (const Container & c) {
        { c.get() } -> pointer;
    };

    // POD or arithmetic type
    template <typename T>
    concept pod = std::is_pod_v<T>;

    // Dereferencable
    template <typename T, typename R>
    concept iterator = requires (T c) {
        { *c } -> std::convertible_to<R>;
        { c++ };
        { ++c };
    };
}
