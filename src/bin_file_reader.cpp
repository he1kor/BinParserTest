#include "bin_file_reader.h"
#include <iostream>

BinFileReader::BinFileReader(std::ifstream& file) : file(file){}

std::expected<void, ifstreamBinaryError> BinFileReader::skip(size_t offset){
    auto fileStatus = checkValidity();
    if (!fileStatus)
        return fileStatus;

    std::streampos originalPos = file.tellg();
    file.seekg(offset, std::ios::cur);

    fileStatus = checkValidity();
    if (!fileStatus)
        return fileStatus;

    if (file.tellg() - originalPos != offset)
        return std::unexpected(ifstreamBinaryError::OUT_OF_RANGE);
    return {};
}

std::expected<void, ifstreamBinaryError> BinFileReader::checkValidity() const{
    if (!file.is_open()){
        return std::unexpected(ifstreamBinaryError::NOT_OPEN);
    }
    if (!file.good()) {
        return std::unexpected(ifstreamBinaryError::ERROR_FILE);
    }
    return {};
}

std::streampos BinFileReader::getCursor() const{
    return file.tellg();
}
