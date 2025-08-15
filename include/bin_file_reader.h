#ifndef BINARY_READER_H
#define BINARY_READER_H

#include <fstream>
#include <array>
#include <expected>
#include <stdint.h>
#include <vector>

#include <algorithm>

#include <iostream>

#include <cstring>

enum class ifstreamBinaryError{
    NOT_OPEN,
    ERROR_FILE,
    OUT_OF_RANGE,
    MODUL_NOT_FOUND
};

enum class Endian {
    Big,
    Little
};

inline const char* ifstreamBinaryErrorStr(ifstreamBinaryError error){
    switch (error){
        case ifstreamBinaryError::NOT_OPEN:
            return "NOT_OPEN\n";
        case ifstreamBinaryError::ERROR_FILE:
            return "ERROR_FILE\n";
        case ifstreamBinaryError::OUT_OF_RANGE:
            return "OUT_OF_RANGE\n";
        default:
            return "UNKNOWN ERROR\n";
    }
}



class BinFileReader{
public:
    BinFileReader() = delete;
    BinFileReader(std::ifstream& file);
    std::expected<void, ifstreamBinaryError> skip(size_t size);

    template <typename T, size_t buffer_size = 1024>
    std::expected<void, ifstreamBinaryError> skipToSignature(T signature);

    template <std::endian Endianess = std::endian::native>
    std::expected<void, ifstreamBinaryError> tryReadData(char* data, size_t size);

    template <std::endian Endianess = std::endian::native>
    std::expected<void, ifstreamBinaryError> tryReadData(std::byte* data, size_t size);

    template <std::endian Endianess = std::endian::native>
    std::expected<std::vector<std::byte>, ifstreamBinaryError> tryReadData(size_t size);
    
    template <typename T, std::endian Endianess = std::endian::native>
    std::expected<T, ifstreamBinaryError> tryReadData();

    std::expected<void, ifstreamBinaryError> checkValidity() const;
    std::streampos getCursor() const;
private:
    std::ifstream& file;
};

template <typename T, size_t buffer_size>
inline std::expected<void, ifstreamBinaryError> BinFileReader::skipToSignature(T signature){
    auto fileStatus = checkValidity();
    if (!fileStatus)
        return fileStatus;


    static_assert(sizeof(T) < buffer_size);

    std::array<char, buffer_size> buffer;
    while (file){
        file.read(buffer.data(), buffer_size);
        auto validness = checkValidity();
        if (!validness)
            return validness;

        size_t bytesRead = file.gcount();

        for (size_t i = 0; i <= bytesRead - sizeof(T); i++){
            if (std::memcmp(buffer.data() + i, reinterpret_cast<char*>(&signature), sizeof(T)) == 0){
                file.seekg(static_cast<std::streamoff>(i) - buffer_size + sizeof(T), std::ios::cur);
                return {};
            }
        }
        if (bytesRead = buffer_size)
            file.seekg(-static_cast<std::streamoff>(sizeof(T) - 1), std::ios::cur);
    }
    return {};
}

template <std::endian Endianess>
inline std::expected<void, ifstreamBinaryError> BinFileReader::tryReadData(char* data, size_t size){
    auto fileStatus = checkValidity();
    if (!fileStatus)
        return fileStatus;
    
    file.read(data, size);
    if (file.gcount() != size)
    return std::unexpected(ifstreamBinaryError::OUT_OF_RANGE);
    if constexpr (Endianess != std::endian::native)
        std::reverse(data, data + size);
    return {};
}

template <std::endian Endianess>
inline std::expected<void, ifstreamBinaryError> BinFileReader::tryReadData(std::byte *data, size_t size){
    return tryReadData<Endianess>(reinterpret_cast<char*>(data), size);
}

template <std::endian Endianess>
inline std::expected<std::vector<std::byte>, ifstreamBinaryError> BinFileReader::tryReadData(size_t size){
    std::vector<std::byte> data(size);
    auto expectedVoid = tryReadData<Endianess>(data.data(), size);
    if (!expectedVoid)
        return std::unexpected(expectedVoid.error());

    return data;
}



template <typename T, std::endian Endianess>
inline std::expected<T, ifstreamBinaryError> BinFileReader::tryReadData(){
    T t;
    auto result = tryReadData<Endianess>(reinterpret_cast<char*>(&t), sizeof(T));
    if (!result.has_value())
        return std::unexpected(result.error());
    return t;
}

#endif //BINARY_READER_H