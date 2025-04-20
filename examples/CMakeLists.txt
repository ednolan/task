# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES
    any_scheduler
    c++now-basic
    c++now-result-types
    c++now-errors
    affinity
    escaped-exception
    co_await-result
    friendly
    loop
    stop
    alloc
    container
    hello
    query
    async-lock
    error
    into_optional
    result_example
)

message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(beman.task.examples.${example})
    target_sources(beman.task.examples.${example} PRIVATE ${example}.cpp)
    target_link_libraries(beman.task.examples.${example} beman::task)
    add_test(
        NAME beman.task.examples.${example}
        COMMAND $<TARGET_FILE:beman.task.examples.${example}>
    )
endforeach()
