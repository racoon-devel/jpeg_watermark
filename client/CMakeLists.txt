cmake_minimum_required(VERSION 3.9)

set(SOURCES
        src/main.cpp
        src/client.cpp
        )

add_executable(jpeg_watermark_client ${SOURCES})
target_include_directories(jpeg_watermark_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_dependencies(jpeg_watermark_client easylogging cxxopts asio)
target_link_libraries(jpeg_watermark_client Threads::Threads easylogging cxxopts asio)