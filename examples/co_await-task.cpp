// examples/co_await-task.cpp                                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/execution/task.hpp>
#include <beman/execution/execution.hpp>
#include <iostream>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

auto inner() -> ex::task<> {
    beman::task::detail::logger log("inner");
    co_return;
}

auto outer() -> ex::task<> {
    beman::task::detail::logger log("outer");
    for (int i{}; i < 10; ++i) {
        co_await inner();
        log.log("inner_awaited");
    }
    co_return;
}

auto main() -> int {
    std::cout << std::unitbuf;
    beman::task::detail::logger log("sync_wait");
    ex::sync_wait(outer());
}
