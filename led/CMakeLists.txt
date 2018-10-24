cmake_minimum_required(VERSION 3.10)
project(ledlib)
set(CMAKE_CXX_STANDARD 11)

#add_library(ledlib SHARED Led.cpp)
add_library(ledlib STATIC Led.cpp)