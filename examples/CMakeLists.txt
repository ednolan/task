# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES
    affinity
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
    add_executable(beman.lazy.examples.${example})
    target_sources(beman.lazy.examples.${example} PRIVATE ${example}.cpp)
    target_link_libraries(beman.lazy.examples.${example} beman::lazy)
endforeach()
