// examples/c++now-cancel.cpp                                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// ----------------------------------------------------------------------------

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------
ex::task<> cancel() {
    co_await ex::just_stopped();
}

int main() {
    ex::sync_wait(cancel());
}
