cmake_minimum_required(VERSION 3.11)

project(postgres CXX)
set(CMAKE_CXX_STANDARD 20)

add_definitions(-DBOOST_BEAST_USE_STD_STRING_VIEW)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(db_example main.cpp)
target_link_libraries(db_example PRIVATE CONAN_PKG::libpqxx CONAN_PKG::boost)
