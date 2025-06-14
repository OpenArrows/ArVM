add_executable(arvm_test_analyze analyze.c)
target_link_libraries(arvm_test_analyze arvm unity)
add_test(NAME ArVM_analyze COMMAND arvm_test_analyze)

add_executable(arvm_test_builder builder.c)
target_link_libraries(arvm_test_builder arvm unity)
add_test(NAME ArVM_builder COMMAND arvm_test_builder)

add_executable(arvm_test_match match.c)
target_link_libraries(arvm_test_match arvm unity)
add_test(NAME ArVM_match COMMAND arvm_test_match)

add_executable(arvm_test_transform transform.c)
target_link_libraries(arvm_test_transform arvm unity)
add_test(NAME ArVM_transform COMMAND arvm_test_transform)

add_executable(arvm_test_opt opt.c)
target_link_libraries(arvm_test_opt arvm unity)
add_test(NAME ArVM_opt COMMAND arvm_test_opt)

add_executable(arvm_test_eval eval.c)
target_link_libraries(arvm_test_eval arvm unity)
add_test(NAME ArVM_eval COMMAND arvm_test_eval)

add_executable(arvm_test_asm asm.c)
target_link_libraries(arvm_test_asm arvm unity)
add_test(NAME ArVM_asm COMMAND arvm_test_asm)