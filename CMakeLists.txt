cmake_minimum_required(VERSION 3.16)
project(chesster LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable high optimization for Release builds
# DNDEBUG (disables assert() checks) allows great optimization for the compiler just need to make sure with the prof that this is allowed 
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG") 
# just have this for now "-O3" maximizes optimization of compiler 
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_BUILD_TYPE Release)

add_executable(chesster
    src/main.cpp
    src/board.cpp
    src/bitboards.cpp
    src/magic.cpp
    src/evaluation.cpp
    src/search.cpp
)
target_include_directories(chesster PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(victory_test
    src/victory_regression.cpp
    src/board.cpp
    src/bitboards.cpp
    src/magic.cpp
    src/evaluation.cpp
    src/search.cpp
)
target_include_directories(victory_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
