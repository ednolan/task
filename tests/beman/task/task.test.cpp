// tests/beman/task/task.test.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>
#include <cassert>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

int main() {
    auto rc = ex::sync_wait([]() -> ex::task<int> { co_return 17; }());
    assert(rc);
    [[maybe_unused]] auto [value] = rc.value_or(std::tuple{0});
    assert(value == 17);
}
