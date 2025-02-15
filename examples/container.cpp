// examples/container.cpp                                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/task.hpp>
#include <beman/execution/execution.hpp>
#include <iostream>
#include <vector>
#include <cassert>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

int main() {
    std::vector<ex::task<>> cont;
    cont.emplace_back([]() -> ex::task<> { co_return; }());
    cont.push_back([]() -> ex::task<> { co_return; }());
}
