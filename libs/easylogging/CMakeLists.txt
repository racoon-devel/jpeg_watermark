cmake_minimum_required(VERSION 3.9)

set (SOURCES 
    easylogging++.cc
)

add_library(easylogging STATIC ${SOURCES})

target_compile_definitions(easylogging PUBLIC -DELPP_THREAD_SAFE)