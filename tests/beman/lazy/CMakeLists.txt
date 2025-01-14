# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

add_executable(beman.lazy.tests.lazy)
target_sources(beman.lazy.tests.lazy PRIVATE lazy.test.cpp)
target_link_libraries(beman.lazy.tests.lazy PRIVATE beman::lazy)
add_test(
    NAME beman.lazy.tests.lazy
    COMMAND $<TARGET_FILE:beman.lazy.tests.lazy>
)
