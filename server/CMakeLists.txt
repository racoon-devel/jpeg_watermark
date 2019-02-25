cmake_minimum_required(VERSION 3.9)

set(SOURCES
    src/main.cpp
    src/server.cpp
    src/task.cpp
    src/threadpool.cpp
)

add_executable(jpeg_watermark_server ${SOURCES})

target_include_directories(jpeg_watermark_server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(jpeg_watermark_server Threads::Threads)
