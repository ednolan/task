// include/beman/task/detail/awaiter.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_AWAITER
#define INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_AWAITER

#include <beman/task/detail/handle.hpp>
#include <beman/task/detail/state_base.hpp>
#include <coroutine>
#include <utility>
#include <iostream> //-dk:TODO

// ----------------------------------------------------------------------------

namespace beman::task::detail {
template <typename Env, typename Promise>
class awaiter : public ::beman::task::detail::state_base<Env> {
  public:
    using stop_token_type = typename ::beman::task::detail::state_base<Env>::stop_token_type;
    using scheduler_type  = typename ::beman::task::detail::state_base<Env>::scheduler_type;

    explicit awaiter(::beman::task::detail::handle<Promise> h) : handle(::std::move(h)) {}
    constexpr auto await_ready() const noexcept -> bool { return false; }
    template <typename PP>
    auto await_suspend(::std::coroutine_handle<PP> parent) noexcept {
        ::beman::task::detail::logger log("awaiter::suspend");
        this->scheduler.emplace(
            this->template from_env<scheduler_type>(::beman::execution::get_env(parent.promise())));
        this->parent = ::std::move(parent);
        assert(this->parent);
        return this->handle.start(this);
    }
    auto await_resume() { ::beman::task::detail::logger l("awaiter::await_resume()"); }
    auto parent_handle() -> ::std::coroutine_handle<> { return ::std::move(this->parent); }

  private:
    auto do_complete() -> std::coroutine_handle<> override {
        ::beman::task::detail::logger l("awaiter::complete()");
        assert(this->parent);
        return ::std::move(this->parent);
    }
    auto do_get_scheduler() -> scheduler_type override { return *this->scheduler; }
    auto do_set_scheduler(scheduler_type other) -> scheduler_type override {
        return ::std::exchange(*this->scheduler, other);
    }
    auto do_get_stop_token() -> stop_token_type override { return {}; }
    auto do_get_context() -> Env& override { return this->env; }

    Env                                    env;
    ::std::optional<scheduler_type>        scheduler;
    ::beman::task::detail::handle<Promise> handle;
    ::std::coroutine_handle<>              parent{};
};
} // namespace beman::task::detail

// ----------------------------------------------------------------------------

#endif
