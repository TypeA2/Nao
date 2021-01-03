#pragma once

#include <type_traits>

namespace nao {
    template <typename T>
    concept member_function_pointer = std::is_member_function_pointer_v<T>;

    
}
