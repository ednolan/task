// examples/result_example.cpp                                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/lazy.hpp>
#include <beman/execution26/execution.hpp>
#include <cassert>

namespace ex = beman::execution26;

// ----------------------------------------------------------------------------

int main() {
    ex::sync_wait([] -> ex::lazy<> {
        int result = co_await [] -> ex::lazy<int> { co_return 42; }();
        assert(result == 42);
    }());
}
