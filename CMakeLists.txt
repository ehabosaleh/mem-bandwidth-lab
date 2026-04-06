cmake_minimum_required(VERSION 3.5)
project(ipcMicrobenchmark LANGUAGES C)
set(CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_library(pipe_bench STATIC ${CMAKE_PATH}/src/pipe_bench.c)
add_library(fifo_bench STATIC ${CMAKE_PATH}/src/fifo_bench.c)
add_library(message_queue_bench STATIC ${CMAKE_PATH}/src/mq_bench.c)
add_library(shm_mem_bench STATIC ${CMAKE_PATH}/src/shm_mem_bench.c)
target_compile_definitions(shm_mem_bench PRIVATE _GNU_SOURCE)


target_include_directories(pipe_bench PUBLIC ${CMAKE_PATH}/include)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_PATH}/bin)

add_executable(pipe_bw tests/pipe_main.c)
target_link_libraries(pipe_bw PRIVATE pipe_bench)

add_executable(fifo_server tests/fifo_server.c)
target_link_libraries(fifo_server PRIVATE fifo_bench)

add_executable(fifo_client tests/fifo_client.c)
target_link_libraries(fifo_client PRIVATE fifo_bench)

add_executable(mq_bw tests/mq_main.c)
target_link_libraries(mq_bw PRIVATE message_queue_bench)

add_executable(shm_mem tests/shm_mem_main.c)
target_link_libraries(shm_mem PRIVATE shm_mem_bench)
