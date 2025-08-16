#include "parser.h"

#include <iostream>
#include <iomanip>
#include <format>

std::expected<std::vector<std::byte>, ifstreamBinaryError> findModule(BinFileReader& file, std::array<char, signatureSize> targetModule){
    
    constexpr std::array<std::byte, 2> headerSignature{std::byte(0xAB),std::byte(0xCD)};

    auto expectedVoid = file.skipToSignature(headerSignature);
    if (!expectedVoid)
        return std::unexpected(expectedVoid.error());
    
    std::streampos start = file.getCursor();
    auto dataModuleSize = file.tryReadData<uint16_t, std::endian::big>();
    
    if (!dataModuleSize)
        return std::unexpected(dataModuleSize.error());

    std::expected<std::array<char, signatureSize>, ifstreamBinaryError> currentModule;
    std::expected<uint16_t, ifstreamBinaryError> expectedSize;
    while (true){
        currentModule = file.tryReadData<std::array<char, signatureSize>>();
        if (!currentModule)
            return std::unexpected(currentModule.error());

        expectedSize = file.tryReadData<uint16_t, std::endian::big>();
        if (!expectedSize)
            return std::unexpected(expectedSize.error());
        if (file.getCursor() - start >= *dataModuleSize)
            return std::unexpected(ifstreamBinaryError::MODUL_NOT_FOUND);
        if (currentModule == targetModule)
            break;
        file.skip((*expectedSize) - sizeof(*expectedSize) - sizeof(*currentModule));
    };

    (*expectedSize) -= sizeof(*expectedSize) + sizeof(*currentModule);
    
    auto expectedRawData = file.tryReadData(*expectedSize);
    if (!expectedRawData)
        return std::unexpected(expectedRawData.error());
    
    return *expectedRawData;
}

std::expected<std::vector<TargetMessagePOD>, ParseError> parseTargetMessages(std::span<const std::byte> data){
    std::vector<TargetMessagePOD> result;
    if (data.size_bytes() % sizeof(TargetMessagePOD) != 0)
        return std::unexpected(ParseError::arraySizeMismatch);
    
    result.reserve(data.size_bytes() / sizeof(TargetMessagePOD));
    for (size_t i = 0; i < data.size_bytes(); i+=sizeof(TargetMessagePOD)){
        auto targetMessage = TargetMessagePOD::fromBytes(data);
        if (!targetMessage)
            return std::unexpected(targetMessage.error());
        result.push_back(*targetMessage);
    }
    return result;
}

void printTargetMessage(BinFileReader& file){
    auto expectedData = findModule(file, "MB"_bin_block);
    if (!expectedData){
        std::cout << "Error ocurred:\t" << ifstreamBinaryErrorStr(expectedData.error()) << '\n';
        return;
    }
    auto span = std::span<const std::byte>(expectedData->data(), expectedData->size()-1);
    auto expectedTargetMessages = parseTargetMessages(span);
    
    if (!expectedTargetMessages){
        std::cout << "Error ocurred:\t" << parseErrorStr(expectedTargetMessages.error()) << '\n';
        std::cout << "TargetMessagePOD size:\t" << sizeof(TargetMessagePOD) << "\tRaw data size given:\t" << span.size_bytes() << "\tWhole module size:\t" << expectedData->size() << '\n';
        return;
    }

    for (size_t i = 0; i < expectedTargetMessages->size(); i++){
        expectedTargetMessages->at(i).print();
        std::cout << "-------------------------------------\n";
    }
}

void TargetMessagePOD::print() const{
    auto printLine = [](const std::string& label, auto value) {
        std::cout << std::format("{:<30}\t{}\n", label, value);
    };
    printLine("Goal numbering:", +goalNumbering);
    printLine("Vertical distance:", verticalDistance_DM);
    printLine("Lateral distance:", lateralDistance_DM);
    printLine("Speed (y direction):", speedY_DM);
    printLine("Target Type:", +targetType);
    printLine("Lane Number:", +laneNumber);
    printLine("Front spacing:", frontSpacing_DM);
    printLine("Front time interval:", frontTimeInterval_DM);
    printLine("Speed (x direction):", speedX_DM);
    printLine("Heading angle:", angle_360);
    printLine("Incidents:", +incidents_bitset);
    printLine("Radar Network X-sit Standard:", radarNetworkXSit_DM);
    printLine("Radar Network Y-sit Standard:", radarNetworkYSit_DM);
    printLine("Fill the blind mark:", +fillTheBlindMark);
    printLine("Car length:", +carLength_DM);
    printLine("Car width:", +carWidth_DM);
}

std::expected<TargetMessagePOD, ParseError> TargetMessagePOD::fromBytes(std::span<const std::byte>& data){
    if (data.size_bytes() < sizeof(TargetMessagePOD))
        return std::unexpected(ParseError::dataSizeMismatch);
    TargetMessagePOD result;
    result.goalNumbering = read<uint8_t, std::endian::big>(data);

    result.verticalDistance_DM = read<int16_t, std::endian::big>(data);
    result.lateralDistance_DM = read<int16_t, std::endian::big>(data);
    result.speedY_DM = read<int16_t, std::endian::big>(data);

    result.targetType = read<int8_t, std::endian::big>(data);
    result.laneNumber = read<int8_t, std::endian::big>(data);

    result.frontSpacing_DM = read<int16_t, std::endian::big>(data);
    result.frontTimeInterval_DM = read<int16_t, std::endian::big>(data);
    result.speedX_DM = read<int16_t, std::endian::big>(data);

    result.angle_360 = read<uint16_t, std::endian::big>(data);
    result.incidents_bitset = read<uint8_t, std::endian::big>(data);
    
    result.radarNetworkXSit_DM = read<int32_t, std::endian::big>(data);
    result.radarNetworkYSit_DM = read<int32_t, std::endian::big>(data);

    result.fillTheBlindMark = read<uint8_t, std::endian::big>(data);

    result.carLength_DM = read<uint8_t, std::endian::big>(data); 
    result.carWidth_DM = read<uint8_t, std::endian::big>(data);
    return result;
}
