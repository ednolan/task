// examples/c++now-basic.cpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>
#include <coroutine>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------
ex::task<> basic() { co_await std::suspend_never{}; }

ex::task<> await_sender() {
    co_await ex::just();
    [[maybe_unused]] int n       = co_await ex::just(1);
    [[maybe_unused]] auto [m, b] = co_await ex::just(1, true);
    try {
        co_await ex::just_error(1);
    } catch (int) {
    }
    co_await ex::just_stopped();
}

int main() {
    ex::sync_wait(std::suspend_never{});
    ex::sync_wait(basic());
    ex::sync_wait(await_sender());
}
