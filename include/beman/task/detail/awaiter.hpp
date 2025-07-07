// include/beman/task/detail/awaiter.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_AWAITER
#define INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_AWAITER

#include <beman/task/detail/handle.hpp>
#include <beman/task/detail/state_base.hpp>
#include <coroutine>
#include <utility>

// ----------------------------------------------------------------------------

namespace beman::task::detail {
template <typename Value, typename Env, typename OwnPromise, typename ParentPromise>
class awaiter : public ::beman::task::detail::state_base<Value, Env> {
  public:
    using stop_token_type = typename ::beman::task::detail::state_base<Value, Env>::stop_token_type;
    using scheduler_type  = typename ::beman::task::detail::state_base<Value, Env>::scheduler_type;

    explicit awaiter(::beman::task::detail::handle<OwnPromise> h) : handle(::std::move(h)) {}
    constexpr auto await_ready() const noexcept -> bool { return false; }
    auto           await_suspend(::std::coroutine_handle<ParentPromise> parent) noexcept {
        this->scheduler.emplace(
            this->template from_env<scheduler_type>(::beman::execution::get_env(parent.promise())));
        this->parent = ::std::move(parent);
        assert(this->parent);
        return this->handle.start(this);
    }
    auto await_resume() { return this->result_resume(); }

  private:
    auto do_complete() -> std::coroutine_handle<> override {
        assert(this->parent);
#if 0
        //-dk:TODO
        assert(this->scheduler);
        if (*this->scheduler != ::beman::execution::get_scheduler(::beman::execution::get_env(this->parent))) {
            assert(nullptr == "shouldn't come here, yet");
        }
#endif
        return this->no_completion_set() ? this->parent.promise().unhandled_stopped() : ::std::move(this->parent);
    }
    auto do_get_scheduler() -> scheduler_type override { return *this->scheduler; }
    auto do_set_scheduler(scheduler_type other) -> scheduler_type override {
        return ::std::exchange(*this->scheduler, other);
    }
    auto do_get_stop_token() -> stop_token_type override { return {}; }
    auto do_get_environment() -> Env& override { return this->env; }

    Env                                    env;
    ::std::optional<scheduler_type>        scheduler;
    ::beman::task::detail::handle<OwnPromise> handle;
    ::std::coroutine_handle<ParentPromise>    parent{};
};
} // namespace beman::task::detail

// ----------------------------------------------------------------------------

#endif
