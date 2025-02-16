# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

list(
    APPEND
    task_tests
    lazy
    promise_type
    error_types_of
    state_base
    with_error
    sub_visit
    result_type
    promise_base
    completion
    handle
    final_awaiter
    allocator_of
    allocator_support
    any_scheduler
    find_allocator
    inline_scheduler
    task
    poly
    scheduler_of
)

foreach(test ${task_tests})
    add_executable(beman.task.tests.${test})
    target_sources(beman.task.tests.${test} PRIVATE ${test}.test.cpp)
    target_link_libraries(beman.task.tests.${test} PRIVATE beman::task)
    add_test(
        NAME beman.task.tests.${test}
        COMMAND $<TARGET_FILE:beman.task.tests.${test}>
    )
endforeach()
