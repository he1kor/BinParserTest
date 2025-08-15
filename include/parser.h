#ifndef PARSER_H
#define PARSER_H

#include <expected>
#include <vector>
#include <span>
#include <bit>

#include "binary.h"
#include "bin_file_reader.h"

enum class ParseError{
    checkSumMismatch,
    validationError,
    dataSizeMismatch,
    arraySizeMismatch
};
inline const char* parseErrorStr(ParseError error){
    switch (error){
        case ParseError::checkSumMismatch:
            return "checkSumMismatch\n";
        case ParseError::validationError:
            return "validationError\n";
        case ParseError::arraySizeMismatch:
            return "arraySizeMismatch\n";
        default:
            return "UNKNOWN ERROR\n";
    }
}


#pragma pack(push, 1)
struct TargetMessagePOD{
    uint8_t goalNumbering;
    
    int16_t verticalDistance_DM;
    int16_t lateralDistance_DM;
    int16_t speedY_DM;

    int8_t targetType;
    int8_t laneNumber;

    int16_t frontSpacing_DM;
    int16_t frontTimeInterval_DM;
    int16_t speedX_DM;

    uint16_t angle_360;
    uint8_t incidents_bitset;
    
    int32_t radarNetworkXSit_DM;
    int32_t radarNetworkYSit_DM;

    uint8_t fillTheBlindMark;

    uint8_t carLength_DM; 
    uint8_t carWidth_DM;

    void print() const;
    static std::expected<TargetMessagePOD, ParseError> fromBytes(std::span<const std::byte> data);
};
#pragma pack(pop)
static_assert(sizeof(TargetMessagePOD) == 29);

constexpr size_t signatureSize = 2;
inline consteval auto operator""_bin_block(const char* str, size_t len) -> std::array<char, signatureSize> {
    if (len != signatureSize) {
        throw "Binary block literal must be exactly 2 characters long";
    }
    std::array<char, signatureSize> result;
    std::copy_n(str, signatureSize, result.begin());
    return result;
}

std::expected<std::vector<TargetMessagePOD>, ParseError> parseTargetMessages(std::span<const std::byte> data);

std::expected<std::vector<std::byte>, ifstreamBinaryError> findModule(BinFileReader& binary, const std::array<char, signatureSize> moduleName);

void printTargetMessage(BinFileReader& file);

#endif // PARSER_H
