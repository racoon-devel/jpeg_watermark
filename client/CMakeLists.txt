cmake_minimum_required(VERSION 3.9)

set(SOURCES
    main.cpp
    client.cpp
)

add_executable(jpeg_watermark_client ${SOURCES})

add_dependencies(jpeg_watermark_client easylogging cxxopts asio)

target_link_libraries(jpeg_watermark_client Threads::Threads easylogging cxxopts asio)