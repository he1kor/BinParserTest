cmake_minimum_required(VERSION 3.10)

project(TestParser)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/parser.cpp
    src/bin_file_reader.cpp
)