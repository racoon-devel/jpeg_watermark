cmake_minimum_required(VERSION 3.9)

set(SOURCES
    main.cpp
)

add_executable(jpeg_watermark_client ${SOURCES})

target_link_libraries(jpeg_watermark_client Threads::Threads)