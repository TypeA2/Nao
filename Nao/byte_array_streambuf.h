#pragma once

#include <streambuf>

#include "concepts.h"

class byte_array_streambuf : public std::streambuf {
    public:
    byte_array_streambuf(const char* data, size_t size);

    template <concepts::pointer T> requires concepts::pod<std::remove_pointer_t<T>>
        byte_array_streambuf(const T* data, size_t size)
            : byte_array_streambuf(reinterpret_cast<const char*>(data), size * sizeof(std::remove_pointer_t<T>)) { }

    protected:
    pos_type seekoff(off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode) override;
    pos_type seekpos(pos_type pos, std::ios_base::openmode) override;
};
