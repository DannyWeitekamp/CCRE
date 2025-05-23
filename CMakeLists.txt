# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.22)

# Set the project name and version
project(cre VERSION 1.0)

# Specify the CPP standard
set(CMAKE_CXX_STANDARD 23)

# Note: Nanobind thing
set(DEV_MODULE Development.Module)


# Add compiler warnings
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wno-sign-compare -pedantic -fPIC)
endif()

# 
add_compile_options(-Wno-unused-variable
                    -Wno-reorder
)

add_compile_options(-std=c++2a -lstdc++)
# add_compile_options(-g)

# set(CMAKE_BUILD_TYPE Debug)
# add_compile_options(-O0)

# add_compile_options(-O3)

add_subdirectory(external/fmt)
add_subdirectory(external/nanobind)
add_subdirectory(external/nanobind/ext/robin_map)
# add_subdirectory(external/rapidjson)

include_directories(include)


# Packages
find_package(Python 3.8 COMPONENTS Interpreter ${DEV_MODULE} REQUIRED)

# Note: Nanobind thing
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# find_package(unordered_dense CONFIG REQUIRED)

# Find all source files in the current directory
file(GLOB SRCS_CORE "src/core/*.cpp")
file(GLOB SRCS_LOGICAL "src/logical/*.cpp")
file(GLOB SRCS_TRANSFORM "src/transform/*.cpp")
# file(GLOB PY_SOURCES "python/*.cpp")


# Build an application library, 
# (So we don't rebuild the source for each target)
add_library(core_lib ${SRCS_CORE})
target_link_libraries(core_lib fmt::fmt)
target_link_libraries(core_lib tsl::robin_map) # for nanobind

add_library(logical_lib ${SRCS_LOGICAL})
target_link_libraries(logical_lib fmt::fmt)
target_link_libraries(logical_lib tsl::robin_map) # for nanobind

add_library(transform_lib ${SRCS_TRANSFORM})
target_link_libraries(transform_lib fmt::fmt)
target_link_libraries(transform_lib tsl::robin_map) # for nanobind

#  -- main -- 
# add_executable(main main.cpp)
# target_link_libraries(main PRIVATE app_lib)

#  -- main2 -- 
# add_executable(main2 main2.cpp)
# target_link_libraries(main2 PRIVATE app_lib)

# ---------- TESTS ----------
enable_testing()

#  -- test_hash -- 
add_executable(test_hash tests/test_hash.cpp)
target_link_libraries(test_hash PRIVATE core_lib)
add_test(
  NAME test_hash
  COMMAND $<TARGET_FILE:test_hash>
  )

add_executable(test_fact tests/test_fact.cpp)
target_link_libraries(test_fact PRIVATE core_lib)
add_test(
  NAME test_fact
  COMMAND $<TARGET_FILE:test_fact>
  )
  
add_executable(test_factset tests/test_factset.cpp)
target_link_libraries(test_factset PRIVATE core_lib transform_lib)
add_test(
  NAME test_factset
  COMMAND $<TARGET_FILE:test_factset>
  )

add_executable(test_func tests/test_func.cpp)
target_link_libraries(test_func PRIVATE core_lib logical_lib)
add_test(
  NAME test_func
  COMMAND $<TARGET_FILE:test_func>
  )

add_executable(test_logical tests/test_logical.cpp)
target_link_libraries(test_logical PRIVATE core_lib logical_lib)
add_test(
  NAME test_logical
  COMMAND $<TARGET_FILE:test_logical>
  )

add_executable(test_transform tests/test_transform.cpp)
target_link_libraries(test_transform PRIVATE core_lib transform_lib)
add_test(
  NAME test_transform
  COMMAND $<TARGET_FILE:test_transform>
  )

nanobind_add_module(dummy_ext python/ext/dummy_ext.cpp)
target_link_libraries(dummy_ext PRIVATE core_lib logical_lib transform_lib)

nanobind_add_module(dummer_ext python/ext/dummer_ext.cpp)
target_link_libraries(dummer_ext PRIVATE core_lib logical_lib transform_lib)


file(GLOB PY_SOURCES "python/src/*.cpp")

# -------- Start Nanobind Module  ---------

nanobind_add_module(cre STABLE_ABI python/ext/cre_py_ext.cpp ${PY_SOURCES})
target_link_libraries(cre PRIVATE core_lib logical_lib transform_lib)
# target_compile_options(cre PRIVATE -g -O0)
# target_compile_options(cre PRIVATE -g -O3)

# target_compile_options(cre PRIVATE $<$<CONFIG:Debug>:${CMAKE_CXX_FLAGS_DEBUG}> $<$<CONFIG:Release>:${CMAKE_CXX_FLAGS_RELEASE}>)

install(TARGETS cre LIBRARY DESTINATION cre)
