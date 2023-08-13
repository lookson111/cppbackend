cmake_minimum_required(VERSION 3.11)

project(tv CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

add_executable(tv
    src/main.cpp
    src/tv.h
    src/tv.cpp
)

add_executable(tests
    tests/tests.cpp
    src/tv.h
    src/tv.cpp
)
target_link_libraries(tests PRIVATE CONAN_PKG::catch2)
