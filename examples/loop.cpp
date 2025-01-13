// examples/loop.cpp                                                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <iostream>
#include <beman/execution26/execution.hpp>
#include <beman/lazy/lazy.hpp>

namespace ex = beman::execution26;

ex::lazy<void> loop() {
    for (int i{}; i < 1000000; ++i)
        co_await ex::just(i);
}

int main() {
    ex::sync_wait(
        // ex::detail::write_env(
        [] -> ex::lazy<void> {
            for (int i{}; i < 1000000; ++i)
                co_await ex::just(i);
        }()
        //, ex::detail::make_env(ex::get_scheduler, ex::inline_scheduler{}))
    );
}
