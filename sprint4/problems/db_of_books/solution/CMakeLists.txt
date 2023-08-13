cmake_minimum_required(VERSION 3.11)

project(postgres CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(BOOST_LIB CONAN_PKG::boost)
set(PQXX_LIB CONAN_PKG::libpqxx)

add_executable(book_manager 
    main.cpp
    json_books.cpp
    json_books.h
    db_books.cpp
    db_books.h)
target_link_libraries(book_manager ${PQXX_LIB} ${BOOST_LIB})
