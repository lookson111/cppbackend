cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)
add_definitions(-DBOOST_BEAST_USE_STD_STRING_VIEW)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(BOOST_LIB CONAN_PKG::boost)
set(ZLIB_LIB Threads::Threads)
set(CATCH2LIB CONAN_PKG::catch2)

# Добавляем библиотеку, указывая, что она статическая.
add_library(ModelLib STATIC     
	src/model.h
    src/model.cpp
    src/dog.h
    src/dog.cpp
    src/extra_data.h
    src/extra_data.cpp
    src/loot_generator.h
    src/loot_generator.cpp
    src/tagged.h
	src/sdk.h
)

# Добавляем сторонние библиотеки. Указываем видимость PUBLIC, т. к. 
# они должны быть ввидны и в библиотеке ModelLib и в зависимостях.
target_include_directories(ModelLib PUBLIC ${BOOST_LIB})
target_link_libraries(ModelLib PUBLIC ${BOOST_LIB} ${ZLIB_LIB})


add_executable(game_server
	src/main.cpp
	src/http_server.cpp
	src/http_server.h
	src/sdk.h
	src/tagged.h
	src/boost_json.cpp
	src/json_loader.h
	src/json_loader.cpp
	src/request_handler/request_handler.cpp
	src/request_handler/request_handler.h
	src/request_handler/api_request.cpp
	src/request_handler/api_request.h
	src/request_handler/file_request.cpp
	src/request_handler/file_request.h
	src/request_handler/base_request.cpp
	src/request_handler/base_request.h
	src/request_handler/response.cpp
	src/request_handler/response.h
	src/request_handler/uri_api.h
	src/request_handler/defs.h
	src/log.cpp
	src/log.h
	src/app.cpp
	src/app.h
	src/token.h
	src/ticker.h
)
#target_include_directories(game_server PRIVATE ${BOOST_LIB})
target_link_libraries(game_server PRIVATE ${BOOST_LIB} ${ZLIB_LIB} ModelLib) 

add_executable(game_server_tests
    tests/model_tests.cpp
    tests/loot_generator_tests.cpp
)
#target_include_directories(game_server_tests PRIVATE ModelLib ${BOOST_LIB})
target_link_libraries(game_server_tests PRIVATE ${CATCH2LIB} ${BOOST_LIB} ${ZLIB_LIB} ModelLib) 

#target_link_libraries(game_server ModelLib)
#target_link_libraries(game_server_tests ${CATCH2_LIB} ModelLib) 