// examples/c++now-errors.cpp                                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>
#include <expected>
#include <iostream>
#include <print>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

//struct none_t {};
using none_t = int;

template <typename...> struct identity_or_none;
template <> struct identity_or_none<> { using type = none_t; };
template <> struct identity_or_none<void> { using type = none_t; };
template <typename T> struct identity_or_none<T> { using type = T; };
template <typename... T>
using identity_or_none_t = typename identity_or_none<T...>::type;

template <ex::sender Sender>
auto as_expected(Sender&& sndr) {
    using value_type = ex::value_types_of_t<
        Sender, ex::empty_env, std::tuple, identity_or_none_t>;
    using error_type = ex::error_types_of_t<
        Sender, ex::empty_env, identity_or_none_t>;
    using result_type = std::expected<value_type, error_type>;

    return std::forward<Sender>(sndr)
        |  ex::then([]<typename T>(T&& x) noexcept {
            return result_type(std::forward<T>(x));
           })
        |  ex::upon_error([]<typename T>(T&& x) noexcept {
            return result_type(std::unexpected(std::forward<T>(x)));
           })
        ;
}

// ----------------------------------------------------------------------------

ex::task<> error_result() {
    try { co_await ex::just_error(17); }
    catch (int n) { std::print("Error: {}\n", n); }
}

void print_expecte(auto const& msg, auto const& e) {
    if (e)
       std::print("{} (value){}\n", msg, e.value());
    else
       std::print("{} (error){}\n", msg, e.error());
}

ex::task<> expected() {
    [[maybe_unused]] auto e = co_await as_expected(ex::just(17));
    print_expecte("expected with value=", e);
    [[maybe_unused]] auto u = co_await as_expected(ex::just_error(17));
    print_expecte("expected without value=", u);
}

int main() {
    ex::sync_wait(error_result());
    ex::sync_wait(expected());
}
