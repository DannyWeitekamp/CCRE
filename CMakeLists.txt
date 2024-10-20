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

# 
add_compile_options(-Wno-unused-variable
                    -Wno-reorder
)

add_compile_options(-std=c++2a -lstdc++)
add_compile_options(-g)
add_compile_options(-O3)

add_subdirectory(external/fmt)
# add_subdirectory(external/rapidjson)


# Packages
# find_package(unordered_dense CONFIG REQUIRED)

# Find all source files in the current directory
file(GLOB SOURCES "src/*.cpp")

# Build an application library, 
# (So we don't rebuild the source for each target)
add_library(app_lib ${SOURCES})
target_link_libraries(app_lib fmt::fmt)

#  -- main -- 
add_executable(main main.cpp)
target_link_libraries(main PRIVATE app_lib)

#  -- main2 -- 
add_executable(main2 main2.cpp)
target_link_libraries(main2 PRIVATE app_lib)

# ---------- TESTS ----------
enable_testing()

#  -- test_hash -- 
add_executable(test_hash tests/test_hash.cpp)
target_link_libraries(test_hash PRIVATE app_lib)
add_test(
  NAME test_hash
  COMMAND $<TARGET_FILE:test_hash>
  )

add_executable(test_factset tests/test_factset.cpp)
target_link_libraries(test_factset PRIVATE app_lib)
add_test(
  NAME test_factset
  COMMAND $<TARGET_FILE:test_factset>
  )

add_executable(test_logical tests/test_logical.cpp)
target_link_libraries(test_logical PRIVATE app_lib)
add_test(
  NAME test_logical
  COMMAND $<TARGET_FILE:test_logical>
  )

add_executable(test_transform tests/test_transform.cpp)
target_link_libraries(test_transform PRIVATE app_lib)
add_test(
  NAME test_transform
  COMMAND $<TARGET_FILE:test_transform>
  )

