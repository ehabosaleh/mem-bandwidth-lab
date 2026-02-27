cmake_minimum_required(VERSION 3.5)
project(ipcMicrobenchmark LANGUAGES C)
set(CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_library(pipe_bench STATIC ${CMAKE_PATH}/src/pipe_bench.c)
target_include_directories(pipe_bench PUBLIC ${CMAKE_PATH}/include)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_PATH}/bin)

add_executable(pipe_bw tests/pipe_main.c)
target_link_libraries(pipe_bw PRIVATE pipe_bench)
