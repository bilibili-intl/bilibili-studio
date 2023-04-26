cmake_minimum_required(VERSION 3.16)
project(potrace VERSION 1.16)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(CheckIncludeFile)

execute_process(COMMAND "${CMAKE_COMMAND}"
    "-E" "tar" "zxf" "${CMAKE_CURRENT_SOURCE_DIR}/potrace-1.16.tar.gz"
    WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(config.h ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_library(potrace STATIC)
target_sources(potrace PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src/bbox.c
    ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src/curve.c
    ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src/decompose.c
    ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src/flate.c
    ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src/greymap.c
    ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src/lzw.c
    ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src/potracelib.c
    ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src/trace.c
    ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src/trans.c
)
target_include_directories(potrace PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(potrace PRIVATE -DHAVE_CONFIG_H=1)

install(TARGETS potrace DESTINATION lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/potrace-1.16/src/potracelib.h DESTINATION include)
