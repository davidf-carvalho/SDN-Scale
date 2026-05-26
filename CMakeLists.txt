cmake_minimum_required(VERSION 3.16)
project(sdn-avl-vs-redblack CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Main executable
add_executable(main
    src/main.cpp
    src/avl/AVLTree.cpp
    src/rbt/RedBlackTree.cpp
)
target_include_directories(main PRIVATE src)

enable_testing()

find_package(GTest CONFIG QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(tests
    tests/test_trees.cpp
    src/avl/AVLTree.cpp
    src/rbt/RedBlackTree.cpp
)
target_include_directories(tests PRIVATE src)
target_link_libraries(tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
