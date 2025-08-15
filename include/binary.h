#ifndef BINARY_H
#define BINARY_H

#include <bit>
#include <cstring>

template <typename T, std::endian Endianess = std::endian::native>
inline T read(std::span<const std::byte>& data){
    T t;
    std::memcpy(&t, data.data(), sizeof(T));
    if constexpr (Endianess != std::endian::native){
        t = std::byteswap(t);
    }
    data = data.subspan(sizeof(T));
    return t;
}

#endif //BINARY_H