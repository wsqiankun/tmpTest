cmake_minimum_required(VERSION 3.10)
project(keylib)
set(CMAKE_CXX_STANDARD 11)

#add_library(keylib SHARED Keys.cpp)
add_library(keylib STATIC Key.cpp)
target_link_libraries(keylib pthread rt)