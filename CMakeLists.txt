cmake_minimum_required(VERSION 3.5)
project(ipcMicrobenchmark LANGUAGES C)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${PROJECT_ROOT}/include)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_ROOT}/bin)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBIBVERBS REQUIRED libibverbs)

add_library(pipe_bench STATIC ${PROJECT_ROOT}/src/pipe_bench.c)
add_library(fifo_bench STATIC ${PROJECT_ROOT}/src/fifo_bench.c)
add_library(message_queue_bench STATIC ${PROJECT_ROOT}/src/mq_bench.c)
add_library(shm_mem_bench STATIC ${PROJECT_ROOT}/src/shm_mem_bench.c)
target_compile_definitions(shm_mem_bench PRIVATE _GNU_SOURCE)
add_library(socket_bench STATIC ${PROJECT_ROOT}/src/socket_bench.c)
add_library(dma_bench STATIC ${PROJECT_ROOT}/src/dma_bench.c)

target_include_directories(pipe_bench PUBLIC ${INCLUDE_DIR})
add_executable(pipe_bw tests/pipe_main.c)
target_link_libraries(pipe_bw PRIVATE pipe_bench)

target_include_directories(fifo_bench PUBLIC ${INCLUDE_DIR})
add_executable(fifo_server tests/fifo_server.c)
target_link_libraries(fifo_server PRIVATE fifo_bench)

add_executable(fifo_client tests/fifo_client.c)
target_link_libraries(fifo_client PRIVATE fifo_bench)

target_include_directories(message_queue_bench PUBLIC ${INCLUDE_DIR})
add_executable(mq_bw tests/mq_main.c)
target_link_libraries(mq_bw PRIVATE message_queue_bench)
target_link_libraries(message_queue_bench PRIVATE rt)


target_include_directories(shm_mem_bench PUBLIC ${INCLUDE_DIR})
add_executable(shm_mem tests/shm_mem_main.c)
target_link_libraries(shm_mem PRIVATE shm_mem_bench)
target_link_libraries(shm_mem PRIVATE shm_mem_bench rt)

target_include_directories(socket_bench PUBLIC ${INCLUDE_DIR})
add_executable(socket_client tests/socket_client.c)
target_link_libraries(socket_client PRIVATE socket_bench)

target_include_directories(socket_bench PUBLIC ${INCLUDE_DIR})
add_executable(socket_server tests/socket_server.c)
target_link_libraries(socket_server PRIVATE socket_bench)

target_include_directories(dma_bench PUBLIC ${INCLUDE_DIR})
target_link_libraries(dma_bench PUBLIC PkgConfig::LIBIBVERBS socket_bench)

add_executable(dma_server tests/dma_server.c)
add_executable(dma_client tests/dma_client.c)
target_link_libraries(dma_server PRIVATE dma_bench)
target_link_libraries(dma_client PRIVATE dma_bench)