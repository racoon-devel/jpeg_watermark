cmake_minimum_required(VERSION 3.9)

set(SOURCES
    main.cpp
    client.cpp
)

add_executable(jpeg_watermark_client ${SOURCES})

target_include_directories(jpeg_watermark_client PUBLIC "${CMAKE_SOURCE_DIR}/libs/easylogging")
add_dependencies(jpeg_watermark_client easylogging)

target_link_libraries(jpeg_watermark_client Threads::Threads easylogging)