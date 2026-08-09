cmake_minimum_required(VERSION 3.20)

project(InputPlay)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(InputPlay src/main.cpp)