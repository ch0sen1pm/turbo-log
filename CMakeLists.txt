cmake_minimum_required(VERSION 3.16)
project(turbo-log LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain")
endif()

find_package(fmt CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(protobuf CONFIG REQUIRED)

add_subdirectory(logger)
add_subdirectory(example)
add_subdirectory(decode)