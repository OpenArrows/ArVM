set(ARVM_TEST_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(arvm_test arvm.c)
target_link_libraries(arvm_test arvm unity)
target_include_directories(arvm_test PRIVATE ${ARVM_TEST_INCLUDE_DIRECTORIES})
add_test(NAME ArVM COMMAND arvm_test)