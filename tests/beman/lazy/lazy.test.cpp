// tests/beman/lazy/lazy.test.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/lazy.hpp>
#include <beman/execution26/execution.hpp>
#include <cassert>

namespace ex = beman::execution26;

// ----------------------------------------------------------------------------

int main() {
    auto rc = ex::sync_wait([]() -> ex::lazy<int> { co_return 17; }());
    assert(rc);
    auto [value] = rc.value_or(std::tuple{0});
    assert(value == 17);
}
