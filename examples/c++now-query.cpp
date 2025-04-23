// examples/c++now-query.cpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>
#include <iostream>

namespace ex = beman::execution;
namespace exd = beman::execution::detail;

// ----------------------------------------------------------------------------

constexpr struct get_value_t {
    template <typename Env>
        requires requires(const get_value_t& self, const Env& e) {
            e.query(self);
        }
    decltype(auto) operator()(const Env& e) const {
        return e.query(*this);
    }
    constexpr auto query(const ex::forwarding_query_t&) const noexcept
        -> bool { return true; }
} get_value{};

struct context {
    int value{};
    context(auto const& e): value(get_value(e)) {}
    int query(get_value_t const&) const { return this->value; }
};

ex::task<void, context> with_env() {
    [[maybe_unused]] decltype(auto) v = co_await ex::read_env(get_value);
}

struct fancy {
    struct env_base { virtual int get() const = 0; };
    template <typename Env>
    struct env_type : env_base {
        env_type(const Env& e) : env(e) {} Env env;
        int get() const override { return get_value(env); }
    };
    int query(const get_value_t&) const { return this->env.get(); }
    fancy(env_base const& own) : env(own) {} env_base const& env;
};

ex::task<void, fancy> with_fancy_env() {
    [[maybe_unused]] decltype(auto) v = co_await ex::read_env(get_value);
    std::cout << "v=" << v << "\n";
}

int main() {
    ex::sync_wait(exd::write_env(with_env(), exd::make_env(get_value, 17)));
    ex::sync_wait(exd::write_env(with_fancy_env(), exd::make_env(get_value, 17)));
}
