cmake_minimum_required(VERSION 3.15)
project(LittleRobot LANGUAGES CXX)
add_executable(LittleRobot src/main.cpp)
add_subdirectory(raylib)
target_link_libraries(LittleRobot PRIVATE raylib)

