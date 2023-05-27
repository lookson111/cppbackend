cmake_minimum_required(VERSION 3.11)

# Проект называется TCP_UDP_Practice и написан на C++
project(Async_res CXX)
# Исходый код будет компилироваться с поддержкой стандарта С++ 20
set(CMAKE_CXX_STANDARD 20)

# Подключаем сгенерированный скрипт conanbuildinfo.cmake, созданный Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# Выполняем макрос из conanbuildinfo.cmake, который настроит СMake на работу с библиотеками, установленными Conan
conan_basic_setup()

# Добавляем проект
add_executable(asyncRest src/main.cpp src/hamburger.h)

# Просим компоновщик подключить библиотеку для поддержки потоков
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(asyncRest PRIVATE Threads::Threads dl)
