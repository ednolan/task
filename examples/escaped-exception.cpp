// examples/escaped-exception.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>
#include <iostream>

namespace ex = beman::execution;
namespace ts = beman::task;

int main() {
    try {
        ex::sync_wait([]() -> ex::task<int> {
            throw std::runtime_error("error");
            co_return 0;
        }());
        std::cout << "not reached!\n";
    }
    catch (std::exception const& ex) {
        std::cout << "ERROR: " << ex.what() << "\n";
    }
}
