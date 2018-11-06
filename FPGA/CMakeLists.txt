cmake_minimum_required(VERSION 3.10)
project(fpgalib)
set(CMAKE_CXX_STANDARD 11)

#add_library(keylib SHARED Keys.cpp)
add_library(fpgalib STATIC fpga.cpp)
target_link_libraries(fpgalib)