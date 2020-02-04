#pragma once

#include <concepts>
#include <ios>

namespace concepts {

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
}
