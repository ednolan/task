// tests/beman/lazy/task.test.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/task.hpp>
#include <beman/execution/execution.hpp>
#include <cassert>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

int main() {
#if 0
    auto rc = ex::sync_wait([]() -> ex::task<int> { co_return 17; }());
    assert(rc);
    auto [value] = rc.value_or(std::tuple{0});
    assert(value == 17);
#endif
}
