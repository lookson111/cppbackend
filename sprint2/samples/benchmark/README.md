Разместите в этой папке решение задачи
mkdir build_debug && cd build_debug
conan install .. -s compiler.libcxx=libstdc++11 -s build_type=Debug
conan install .. -s build_type=Debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . && bin/hello_log
