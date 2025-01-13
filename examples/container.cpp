// examples/container.cpp                                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/lazy.hpp>
#include <beman/execution26/execution.hpp>
#include <iostream>
#include <vector>
#include <cassert>

namespace ex = beman::execution26;

// ----------------------------------------------------------------------------

int main() {
    std::vector<ex::lazy<>> cont;
    cont.emplace_back([]() -> ex::lazy<> { co_return; }());
    cont.push_back([]() -> ex::lazy<> { co_return; }());
}
