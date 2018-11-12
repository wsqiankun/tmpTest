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

# set(CMAKE_SKIP_BUILD_RPATH FALSE)
# set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
# set(CMAKE_INSTALL_RPATH "")
# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)


# add_subdirectory(common)
add_subdirectory(led)
add_subdirectory(key)
add_subdirectory(usbdetect)
add_subdirectory(FPGA)
add_subdirectory(lcd)
add_executable(zc55proj main.cpp bmp.c)

target_link_libraries(zc55proj ledlib keylib usblib fpgalib lcdlib pthread rt)
set_target_properties(zc55proj PROPERTIES LINK_FLAGS "-Wl,-rpath-link,.")