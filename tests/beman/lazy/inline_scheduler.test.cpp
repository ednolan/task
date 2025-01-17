// tests/beman/lazy/inline_scheduler.test.cpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/detail/inline_scheduler.hpp>
#include <beman/execution26/execution.hpp>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

namespace ex = beman::execution26;
namespace ly = beman::lazy;

// ----------------------------------------------------------------------------

namespace {
struct receiver {
    using receiver_concept = ex::receiver_t;
    int& value;

    void set_value(int v) && noexcept { this->value = v; }
};
static_assert(ex::receiver<receiver>);
} // namespace

int main() {
    ly::detail::inline_scheduler sched;
    static_assert(ex::scheduler<decltype(sched)>);

    auto sched_sender{ex::schedule(sched)};
    static_assert(ex::sender<decltype(sched_sender)>);

    auto env{ex::get_env(sched_sender)};
    assert(sched == ex::get_completion_scheduler<ex::set_value_t>(env));

    int  value{};
    auto state{ex::connect(std::move(sched_sender) | ex::then([]() noexcept { return 17; }), receiver(value))};
    static_assert(ex::operation_state<decltype(state)>);

    assert(value == 0);
    ex::start(state);
    assert(value == 17);
}
