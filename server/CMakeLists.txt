cmake_minimum_required(VERSION 3.9)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

set(SOURCES
        src/main.cpp
        src/service.cpp
        src/server.cpp
        src/session_manager.cpp
        src/session.cpp
        src/worker_pool.cpp
        src/task.cpp
        src/watermark_task.cpp
        )

add_executable(jpeg_watermark_server ${SOURCES})
add_dependencies(jpeg_watermark_server easylogging cxxopts cimg asio)

target_include_directories(jpeg_watermark_server PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(jpeg_watermark_server Threads::Threads easylogging cxxopts cimg asio ${JPEG_LIBRARIES})
