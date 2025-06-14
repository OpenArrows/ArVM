add_library(arvm STATIC
    eval.c builder.c opt.c analyze.c
    asm.c codegen.c xbuf.c
    transform.c match.c visit.c arena.c
    iterators.c imath.c
)
set_property(TARGET arvm PROPERTY C_STANDARD 23)
target_include_directories(arvm PUBLIC ${CMAKE_CURRENT_LIST_DIR})