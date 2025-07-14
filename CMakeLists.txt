# TODO: Makefile

cmake_minimum_required(VERSION 3.10)
project(ArVM
    VERSION 0.2.0
    LANGUAGES C
    DESCRIPTION "A fast CA backend"
)

add_library(arvm STATIC arvm.c bdd.c)
set_property(TARGET arvm PROPERTY C_STANDARD 23)
target_include_directories(arvm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(UNITY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/Unity")
add_subdirectory(${UNITY_PATH})

include(CTest)
enable_testing()

add_subdirectory(test)