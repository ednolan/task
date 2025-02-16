// tests/beman/task/promise_base.test.cpp                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/detail/promise_base.hpp>
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
    void  set_error(auto&&) && noexcept { assert(nullptr == "unexpected call to set_error"); }
};
static_assert(::beman::execution::receiver<void_receiver>);

struct int_receiver {
    using receiver_concept = ::beman::execution::receiver_t;

    int& value;
    void set_value(int v) && noexcept { this->value = v; }
    void set_error(auto&&) && noexcept { assert(nullptr == "unexpected call to set_error"); }
};
static_assert(::beman::execution::receiver<int_receiver>);
} // namespace

int main() {
    {
        beman::task::detail::promise_base<beman::task::detail::stoppable::no,
                                          void,
                                          beman::execution::completion_signatures<beman::execution::set_error_t(int)>>
            result{};
        result.return_void();
        bool flag{false};
        result.complete(void_receiver{flag});
        assert(flag == true);
    }
    {
        beman::task::detail::promise_base<beman::task::detail::stoppable::no,
                                          int,
                                          beman::execution::completion_signatures<beman::execution::set_error_t(int)>>
            result{};
        result.return_value(17);
        int value{};
        result.complete(int_receiver{value});
        assert(value == 17);
    }
}
