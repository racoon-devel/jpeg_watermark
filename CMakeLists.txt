cmake_minimum_required(VERSION 3.9)

project(jpeg_watermark VERSION 1.1)

set(GLOBAL_OUTPUT_PATH ${PROJECT_BINARY_DIR})

option(WITH_ASAN "Build with address sanitizer" OFF)

# set C++14 option
set (CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH}/lib)

# Find threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(JPEG REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")

add_subdirectory(3rdparty)

add_subdirectory(server)

add_subdirectory(client)


