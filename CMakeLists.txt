# TODO: Makefile

cmake_minimum_required(VERSION 3.10)
project(ArVM
    VERSION 0.2.0
    LANGUAGES C
    DESCRIPTION "A fast CA backend"
)

find_package(OpenMP REQUIRED)

add_library(arvm STATIC src/arvm.c src/bdd.c src/util/pool.c src/util/cons.c src/util/hashtable.c)
set_property(TARGET arvm PROPERTY C_STANDARD 99)
set_property(TARGET arvm PROPERTY C_EXTENSIONS OFF)
target_link_libraries(arvm PRIVATE ${OpenMP_C_LIBRARIES})
set_property(TARGET arvm PROPERTY COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:C>:SHELL:${OpenMP_C_FLAGS}>") # workaround
target_include_directories(arvm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(arvm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(UNITY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/Unity")
add_subdirectory(${UNITY_PATH})

include(CTest)
enable_testing()

add_subdirectory(test)