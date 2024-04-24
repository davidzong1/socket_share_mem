# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本
project(MyProject VERSION 1.0)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(./)

add_executable(client_demo 
./main_client.cc
./dz_communicator.cc
)
target_link_libraries(client_demo rt pthread)