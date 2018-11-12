cmake_minimum_required(VERSION 3.10)
project(lcdlib)
set(CMAKE_CXX_STANDARD 11)

#add_library(keylib SHARED Keys.cpp)
add_library(lcdlib STATIC lcd.cpp)
target_link_libraries(lcdlib)