#include <iostream>

#include <fstream>
#include "bin_file_reader.h"
#include "parser.h"

int main(){
    std::ifstream inputFile("test_data.bin", std::ios::binary);
    BinFileReader bin(inputFile);
    printTargetMessage(bin);
}