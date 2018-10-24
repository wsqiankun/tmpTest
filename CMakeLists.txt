cmake_minimum_required(VERSION 3.10)

SET(CMAKE_SYSTEM_NAME Linux)

###### corss compile #########
SET(CROSS_COMPILE 1)

IF(CROSS_COMPILE)

SET(CMAKE_SYSTEM_PROCESSOR arm)

SET(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++")
SET(CMAKE_C_COMPILER   "aarch64-linux-gnu-gcc")


ENDIF(CROSS_COMPILE)
###############################

project(zc55proj)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(led)
add_executable(zc55proj main.cpp)

target_link_libraries(zc55proj ledlib)