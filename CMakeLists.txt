cmake_minimum_required(VERSION 3.16)
project(chesster LANGUAGES CXX)

# Use at least C++17 (C++11 would also be fine, but 17 is safe and modern)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(chesster
    src/main.cpp
    src/board.cpp
    src/bitboards.cpp
)
