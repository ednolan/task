// tests/beman/lazy/completion.test.cpp                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/detail/completion.hpp>
#include <beman/execution/execution.hpp>
#include <concepts>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

int main() {
    struct type {};

    static_assert(std::same_as<beman::lazy::detail::completion_t<void>, ex::set_value_t()>);
    static_assert(std::same_as<beman::lazy::detail::completion_t<int>, ex::set_value_t(int)>);
    static_assert(std::same_as<beman::lazy::detail::completion_t<type>, ex::set_value_t(type)>);
}
