cmake_minimum_required(VERSION 3.9)

project(jpeg_watermark VERSION 1.0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

set(GLOBAL_OUTPUT_PATH ${PROJECT_BINARY_DIR})

# set C++11 option
set (CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH}/lib)

# Find threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")

add_subdirectory("server")

add_subdirectory("client")


