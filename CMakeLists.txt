cmake_minimum_required(VERSION 3.10)
project(ArVM
    VERSION 0.1.0
    LANGUAGES C
    DESCRIPTION "A fast CA backend"
)

add_library(arvm STATIC
    src/arvm.c

    src/arena.c

    src/ir/builder.c
    src/ir/analyze.c
    src/ir/transform.c
    src/ir/visit.c
    src/ir/match.c
    src/ir/opt.c

    src/eval.c

    src/codegen/asm.c
    src/codegen/xbuf.c
)
set_property(TARGET arvm PROPERTY C_STANDARD 23)
target_include_directories(arvm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(arvm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(UNITY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/Unity")
add_subdirectory(${UNITY_PATH})

include(CTest)
enable_testing()

add_subdirectory(test)