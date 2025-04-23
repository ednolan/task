// examples/c++now-with_error.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>
#include <print>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

struct E {
    int value;
};

struct context {
    using error_types = ex::completion_signatures<ex::set_error_t(E)>;
};

// ----------------------------------------------------------------------------

ex::task<int, context> error_return(int arg) noexcept {
    if (arg == 1)
        co_yield ex::with_error(E{arg});
    co_return arg * 17;
}

struct ctxt {
    using error_types = ex::completion_signatures<ex::set_error_t(int)>;
};

ex::task<int, ctxt> call(int v) {
    if (v == 1)
        co_yield ex::with_error(-1);
    co_return 2 * v;
}

int main(int ac, char*[]) {
    for (int i{}; i != 3; ++i) {
        auto [r] = *ex::sync_wait(error_return(i) | ex::upon_error([](auto x) {
                                      std::print("error: {}\n", x.value);
                                      return -1;
                                  }));
        std::print("{}: r={}\n", i, r);
    }

    [[maybe_unused]] auto [n] = *ex::sync_wait(call(ac) | ex::upon_error([](int e) {
                                                   std::print("error({})\n", e);
                                                   return -1;
                                               }));
}
