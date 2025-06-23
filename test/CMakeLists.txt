set(ARVM_TEST_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(arvm_test_analyze analyze.c)
target_link_libraries(arvm_test_analyze arvm unity)
target_include_directories(arvm_test_analyze PRIVATE ${ARVM_TEST_INCLUDE_DIRECTORIES})
add_test(NAME ArVM_analyze COMMAND arvm_test_analyze)

add_executable(arvm_test_builder builder.c)
target_link_libraries(arvm_test_builder arvm unity)
target_include_directories(arvm_test_builder PRIVATE ${ARVM_TEST_INCLUDE_DIRECTORIES})
add_test(NAME ArVM_builder COMMAND arvm_test_builder)

add_executable(arvm_test_match match.c)
target_link_libraries(arvm_test_match arvm unity)
target_include_directories(arvm_test_match PRIVATE ${ARVM_TEST_INCLUDE_DIRECTORIES})
add_test(NAME ArVM_match COMMAND arvm_test_match)

add_executable(arvm_test_transform transform.c)
target_link_libraries(arvm_test_transform arvm unity)
target_include_directories(arvm_test_transform PRIVATE ${ARVM_TEST_INCLUDE_DIRECTORIES})
add_test(NAME ArVM_transform COMMAND arvm_test_transform)

add_executable(arvm_test_opt opt.c)
target_link_libraries(arvm_test_opt arvm unity)
target_include_directories(arvm_test_opt PRIVATE ${ARVM_TEST_INCLUDE_DIRECTORIES})
add_test(NAME ArVM_opt COMMAND arvm_test_opt)

add_executable(arvm_test_print print.c)
target_link_libraries(arvm_test_print arvm unity)
target_include_directories(arvm_test_print PRIVATE ${ARVM_TEST_INCLUDE_DIRECTORIES})
add_test(NAME ArVM_print COMMAND arvm_test_print)

add_executable(arvm_test_eval eval.c)
target_link_libraries(arvm_test_eval arvm unity)
target_include_directories(arvm_test_eval PRIVATE ${ARVM_TEST_INCLUDE_DIRECTORIES})
add_test(NAME ArVM_eval COMMAND arvm_test_eval)