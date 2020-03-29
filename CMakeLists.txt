cmake_minimum_required(VERSION 3.10)
project(ImageEditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/obj)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(ImageEditor main.cpp)

add_library(Image Image.cpp Image.hpp)
add_library(ImageWindow ImageWindow.cpp ImageWindow.hpp)

target_link_libraries(ImageWindow Image)
target_link_libraries(ImageEditor Image ImageWindow)
target_link_libraries(ImageEditor ${CONAN_LIBS})