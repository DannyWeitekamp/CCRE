# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(MyProject VERSION 1.0)

# Specify the CPP standard
set(CMAKE_CXX_STANDARD 23)

# Add compiler warnings
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wno-sign-compare -pedantic)
endif()

add_compile_options(-std=c++2a -lstdc++)

# Find all source files in the current directory
file(GLOB SOURCES "src/*.cpp")

# Create an executable from the source files
add_executable(main main.cpp ${SOURCES})
target_include_directories(main PRIVATE include)
