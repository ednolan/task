// examples/loop.cpp                                                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <iostream>
#include <string>
#include <beman/execution/execution.hpp>
#include <beman/task/task.hpp>

namespace ex = beman::execution;

namespace {
[[maybe_unused]] ex::task<void> loop(auto count) {
    for (int i{}; i < count; ++i)
        co_await ex::just(i);
}
} // namespace

int main(int ac, char* av[]) {
    auto count = 1 < ac && av[1] == std::string_view("run-it") ? 1000000 : 10000;
#if 1
    ex::sync_wait(loop(count));
#else
    ex::sync_wait(ex::detail::write_env(loop(count), ex::detail::make_env(ex::get_scheduler, ex::inline_scheduler{})));
#endif
}
