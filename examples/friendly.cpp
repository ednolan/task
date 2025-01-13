// examples/friendly.cpp                                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <memory>
#include <iostream>
#include <coroutine>
#include <exception>
#include <system_error>
#include <beman/execution26/execution.hpp>
#include <beman/lazy/lazy.hpp>

namespace ex = beman::execution26;

int main() {
    ex::sync_wait([]() -> ex::lazy<void> { co_await ex::just(); }());
    ex::sync_wait([]() -> ex::lazy<void> { co_await std::suspend_never(); }());
}
