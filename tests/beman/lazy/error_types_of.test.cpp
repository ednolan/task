// tests/beman/lazy/detail/error_types_of.test.cpp                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/detail/error_types_of.hpp>
#include <beman/execution/execution.hpp>
#include <concepts>
#include <exception>

// ----------------------------------------------------------------------------

namespace {
struct default_context {};
template <typename... E>
struct error_context {
    using error_types = beman::execution::completion_signatures<beman::execution::set_error_t(E)...>;
};
} // namespace

int main() {
    static_assert(
        std::same_as<beman::lazy::detail::error_types_of_t<default_context>,
                     beman::execution::completion_signatures<beman::execution::set_error_t(std::exception_ptr)>>);
    static_assert(std::same_as<beman::lazy::detail::error_types_of_t<error_context<>>,
                               beman::execution::completion_signatures<>>);
    static_assert(std::same_as<beman::lazy::detail::error_types_of_t<error_context<int, bool>>,
                               beman::execution::completion_signatures<beman::execution::set_error_t(int),
                                                                       beman::execution::set_error_t(bool)>>);
}
