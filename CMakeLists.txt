cmake_minimum_required(VERSION 3.10)
project(ArVM
    VERSION 0.1.0
    LANGUAGES C
    DESCRIPTION "A fast CA backend"
)

add_subdirectory(src)

set(UNITY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unity")
add_subdirectory(${UNITY_PATH})

include(CTest)
enable_testing()

add_subdirectory(test)