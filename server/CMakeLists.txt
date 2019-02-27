cmake_minimum_required(VERSION 3.9)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

set(SOURCES
    src/image_processor.cpp
    src/main.cpp
    src/server.cpp
    src/session.cpp
)

add_executable(jpeg_watermark_server ${SOURCES})

add_dependencies(jpeg_watermark_server easylogging)

target_include_directories(jpeg_watermark_server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(jpeg_watermark_server PUBLIC "${CMAKE_SOURCE_DIR}/libs/easylogging")

target_link_libraries(jpeg_watermark_server Threads::Threads easylogging)
