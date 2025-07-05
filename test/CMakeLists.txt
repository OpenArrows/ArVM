set(ARVM_TEST_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(arvm_test_x x.c)
target_link_libraries(arvm_test_x arvm unity)
target_include_directories(arvm_test_x PRIVATE ${ARVM_TEST_INCLUDE_DIRECTORIES})
add_test(NAME ArVM_x COMMAND arvm_test_x)