// examples/hello.cpp                                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/lazy.hpp>
#include <beman/execution26/execution.hpp>
#include <iostream>

namespace ex = beman::execution26;
namespace ly = beman::lazy;

int main() {
    return std::get<0>(*ex::sync_wait([] -> ex::lazy<int> {
        std::cout << "Hello, world!\n";
        co_return co_await ex::just(0);
    }()));
}
