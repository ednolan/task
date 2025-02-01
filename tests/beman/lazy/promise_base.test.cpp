// tests/beman/lazy/promise_base.test.cpp                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/detail/promise_base.hpp>
#include <beman/execution/execution.hpp>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

// ----------------------------------------------------------------------------

namespace {
struct void_receiver {
    using receiver_concept = ::beman::execution::receiver_t;

    bool& flag;
    void  set_value() && noexcept { flag = true; }
};
static_assert(::beman::execution::receiver<void_receiver>);

struct int_receiver {
    using receiver_concept = ::beman::execution::receiver_t;

    int& value;
    void set_value(int v) && noexcept { this->value = v; }
};
static_assert(::beman::execution::receiver<int_receiver>);
} // namespace

int main() {
    {
        beman::lazy::detail::promise_base<beman::lazy::detail::stoppable::no, void> result{};
        result.return_void();
        bool flag{false};
        result.complete(void_receiver{flag});
        assert(flag == true);
    }
    {
        beman::lazy::detail::promise_base<beman::lazy::detail::stoppable::no, int> result{};
        result.return_value(17);
        int value{};
        result.complete(int_receiver{value});
        assert(value == 17);
    }
}
