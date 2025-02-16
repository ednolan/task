// tests/beman/task/lazy.test.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/lazy.hpp>
#include <beman/execution/execution.hpp>

// ----------------------------------------------------------------------------

int main() {
    beman::execution::sync_wait([]()->beman::lazy::lazy<>{ co_return; }());
    beman::execution::sync_wait([]()->beman::execution::lazy<>{ co_return; }());
}
