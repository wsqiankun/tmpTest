cmake_minimum_required(VERSION 3.10)
project(usblib)
set(CMAKE_CXX_STANDARD 11)

add_library(usblib STATIC UsbDetector.cpp ../common/common.cpp)
target_link_libraries(usblib  pthread rt)