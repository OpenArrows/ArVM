add_library(arvm STATIC
    eval.c builder.c opt.c analyze.c
    transform.c match.c visit.c arena.c imath.c
)
set_property(TARGET arvm PROPERTY C_STANDARD 23)
target_include_directories(arvm PUBLIC ${CMAKE_CURRENT_LIST_DIR})