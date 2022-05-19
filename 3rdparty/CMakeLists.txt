cmake_minimum_required(VERSION 3.9)

# соберем библиотеку логирования, там есть поддержка CMAKE, но она уродская
add_library(easylogging STATIC easyloggingpp/src/easylogging++.cc)
target_include_directories(easylogging PUBLIC easyloggingpp/src)

# используем потокобезопасность в логах
target_compile_definitions(easylogging PUBLIC -DELPP_THREAD_SAFE)

# библиотека для парсинга опций командной строки
add_subdirectory(cxxopts)

# библиотека обработки изображений
add_library(cimg INTERFACE)
target_include_directories(cimg INTERFACE CImg/)
target_compile_definitions(cimg INTERFACE -Dcimg_use_jpeg -Dcimg_use_pthread -Dcimg_plugin="plugins/jpeg_buffer.h" -Dcimg_display=0)

# standalone asio
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include)
target_compile_definitions(asio INTERFACE -DASIO_STANDALONE)