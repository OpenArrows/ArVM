add_library(arvm STATIC
    eval.c builder.c opt.c analyze.c
    visit.c arena.c imath.c
)
target_include_directories(arvm PUBLIC ${CMAKE_CURRENT_LIST_DIR})