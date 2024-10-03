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

# Packages
# find_package(unordered_dense CONFIG REQUIRED)

# Find all source files in the current directory
file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB C_SOURCES "src/*.c")

# Create an executable from the source files
add_executable(main main.cpp ${CPP_SOURCES} ${C_SOURCES})
target_include_directories(main PRIVATE include)
# target_link_libraries(test_hash unordered_dense::unordered_dense)

# --- TESTS -----
enable_testing()

add_executable(test_hash tests/test_hash.cpp ${CPP_SOURCES} ${C_SOURCES})
target_include_directories(test_hash PRIVATE include)
# target_link_libraries(test_hash unordered_dense::unordered_dense)





add_test(
  NAME test_hash
  COMMAND $<TARGET_FILE:test_hash>
  )
