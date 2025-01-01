cmake_minimum_required(VERSION 3.10)

# Project Name
project(MyProject)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)

# Add Executable
add_executable(MCVulkan src/main.cpp)

set_property(TARGET MCVulkan PROPERTY CXX_STANDARD 22)
