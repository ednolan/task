// tests/beman/task/task.test.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>
#include <cassert>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

namespace {
    auto test_co_return() {
    auto rc = ex::sync_wait([]() -> ex::task<int> { co_return 17; }());
    assert(rc);
    [[maybe_unused]] auto [value] = rc.value_or(std::tuple{0});
    assert(value == 17);

    }

    auto test_cancel() {
        auto rc = ex::sync_wait([]() -> ex::task<> {
            bool stopped{};
            co_await (
                []()->ex::task<void> { co_await ex::just_stopped(); }()
                | ex::upon_stopped([&stopped]() { stopped = true; })
            );
            assert(stopped);
        }());
    }

    auto test_indirect_cancel() {
        // This approach uses symmetric transfer
        auto rc = ex::sync_wait([]() -> ex::task<> {
            std::cout << "indirect cancel\n" << std::unitbuf;
            bool stopped{};
            std::cout << "outer co_await\n";
            co_await (
                []()->ex::task<void> {
                    std::cout << "middle co_await\n";
                    co_await []()->ex::task<void> {
                        std::cout << "inner stopping\n";
                        co_await ex::just_stopped();
                        std::cout << "inner after stopped\n";
                    }();
                    std::cout << "middle after co_await\n";
                }()
                | ex::upon_stopped([&stopped]() { stopped = true; })
            );
            std::cout << "outer after co_await\n";
            assert(stopped);
        }());
    }
}

auto main() -> int{
    test_co_return();
    test_cancel();
    test_indirect_cancel();
}
