cmake_minimum_required(VERSION 3.27)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/Toolchain.cmake)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

project(Kernel VERSION 0.1 LANGUAGES ASM C)

set(includes ${CMAKE_SOURCE_DIR}/include)
file(GLOB_RECURSE sources src/*.s src/*.c)

add_executable(kernel ${sources})
target_include_directories(kernel PRIVATE ${includes})

install(TARGETS kernel DESTINATION build/kernel)