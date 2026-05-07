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
