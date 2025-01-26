// examples/result_example.cpp                                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/lazy.hpp>
#include <beman/execution/execution.hpp>
#include <cassert>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

int main() {
    ex::sync_wait([]() -> ex::lazy<> {
        [[maybe_unused]] int result = co_await []() -> ex::lazy<int> { co_return 42; }();
        assert(result == 42);
    }());
}
