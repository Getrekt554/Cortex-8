cmake_minimum_required(VERSION 3.10)

project(compter) 

set(CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-windows")

find_package(raylib REQUIRED)

add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE raylib)