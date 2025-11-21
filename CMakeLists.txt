cmake_minimum_required(VERSION 3.15)
project(pmr_forward_list VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для header-only библиотеки используем INTERFACE библиотеку
add_library(pmr_forward_list INTERFACE)
target_include_directories(pmr_forward_list INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Main executable
add_executable(main src/main.cpp)
target_link_libraries(main pmr_forward_list)

# Tests with GTest (using FetchContent)
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_fl tests/test_fl.cpp)
target_link_libraries(test_fl pmr_forward_list GTest::gtest_main)
add_test(NAME forward_list_test COMMAND test_fl)