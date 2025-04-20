// examples/c++now-with_error.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>
#include <print>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

struct E { int value; };
struct F { int value; };

struct context {
    using error_types = ex::completion_signatures<
        ex::set_error_t(E),
        ex::set_error_t(F)
    >;
};

// ----------------------------------------------------------------------------

ex::task<int, context> error_return(int arg) noexcept {
    if (arg == 1)
        co_yield ex::with_error(E{arg});
    if (arg == 2)
        co_yield ex::with_error(F{arg});
    co_return 17;
}

int main() {
    for (int i{}; i != 3; ++i) {
        auto[r] = *ex::sync_wait(
            error_return(i)
            | ex::upon_error([](auto x){
                auto e{std::same_as<decltype(x), E>? "E": "F"};
                std::print("error: {}={}\n", e, x.value);
                return -1;
            })
        );
        std::print("{}: r={}\n", i, r);
    }
}
