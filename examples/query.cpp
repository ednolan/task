// examples/query.cpp                                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/execution/execution.hpp>
#include <beman/lazy/lazy.hpp>
#include <iostream>
#include <cassert>
#include <cinttypes>

namespace ex = beman::execution;

constexpr struct get_value_t {
    template <typename Env>
        requires requires(const get_value_t& self, const Env& e) { e.query(self); }
    int operator()(const Env& e) const {
        return e.query(*this);
    }
    constexpr auto query(const ::beman::execution::forwarding_query_t&) const noexcept -> bool { return true; }
} get_value{};

struct simple_context {
    int value{};
    int query(const get_value_t&) const noexcept { return this->value; }
    simple_context(auto&& env) : value(get_value(env)) {}
};

struct context {
    template <typename>
    struct env_type {
        int value{};
        env_type(const auto&& env) : value(get_value(env) + 3) {}
    };
    int value{};
    int query(const get_value_t&) const noexcept { return this->value; }
    context(auto& own) : value(own.value) {}
};

int main() {
    ex::sync_wait(ex::detail::write_env(
        []() -> ex::lazy<void, simple_context> {
            auto value(co_await ex::read_env(get_value));
            std::cout << "value=" << value << "\n";
        }(),
        ex::detail::make_env(get_value, 42)));
    ex::sync_wait(ex::detail::write_env(
        []() -> ex::lazy<void, context> {
            auto value(co_await ex::read_env(get_value));
            std::cout << "value=" << value << "\n";
        }(),
        ex::detail::make_env(get_value, 42)));
}
