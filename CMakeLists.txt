cmake_minimum_required(VERSION 3.10)

project(decoder)

include(bench/GoogleBenchmark.cmake)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(unitTest)

file(GLOB DECODE_SRC decoder_low_quality/*.cpp)
file(GLOB DECODE_HEADERS decoder_low_quality/*.h)

add_library(decoder STATIC ${DECODE_SRC} ${DECODE_HEADERS})
add_executable(run main.cpp)

target_link_libraries(run decoder)

include_directories(decoder_low_quality)

# Add source to this project's executable.
add_executable (bench1 bench/bench1.cpp)

# Link Google Benchmark to the project
target_link_libraries(bench1 decoder benchmark::benchmark)
