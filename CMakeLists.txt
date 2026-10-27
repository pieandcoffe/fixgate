cmake_minimum_required(VERSION 3.24)
project(fixgate VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Shared warning set, applied to every compiled target.
add_library(fixgate_warnings INTERFACE)
target_compile_options(fixgate_warnings INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror)

# Header-only codec library: linking it adds include/ to the include path.
add_library(fixgate_codec INTERFACE)
add_library(fixgate::codec ALIAS fixgate_codec)
target_include_directories(fixgate_codec INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(fixgate src/main.cpp)
target_link_libraries(fixgate PRIVATE fixgate::codec fixgate_warnings)

# Unit tests are picked up automatically from the tests/ folder
option(FIXGATE_BUILD_TESTS "Build unit tests" ON)
if(FIXGATE_BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(tests)
endif()
