// include/beman/lazy/detail/promise_type.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_PROMISE_TYPE
#define INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_PROMISE_TYPE

#include <beman/lazy/detail/allocator_of.hpp>
#include <beman/lazy/detail/allocator_support.hpp>
#include <beman/lazy/detail/error_types_of.hpp>
#include <beman/lazy/detail/final_awaiter.hpp>
#include <beman/lazy/detail/find_allocator.hpp>
#include <beman/lazy/detail/handle.hpp>
#include <beman/lazy/detail/inline_scheduler.hpp>
#include <beman/lazy/detail/promise_base.hpp>
#include <beman/lazy/detail/result_type.hpp>
#include <beman/lazy/detail/scheduler_of.hpp>
#include <beman/lazy/detail/state_base.hpp>
#include <beman/lazy/detail/with_error.hpp>
#include <beman/execution/execution.hpp>
#include <coroutine>
#include <optional>
#include <type_traits>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {
template <typename Coroutine, typename T, typename C>
struct promise_type : ::beman::lazy::detail::promise_base<::beman::lazy::detail::stoppable::yes,
                                                          ::std::remove_cvref_t<T>,
                                                          ::beman::lazy::detail::error_types_of_t<C>>,
                      ::beman::lazy::detail::allocator_support<::beman::lazy::detail::allocator_of_t<C>> {
    using allocator_type   = ::beman::lazy::detail::allocator_of_t<C>;
    using scheduler_type   = ::beman::lazy::detail::scheduler_of_t<C>;
    using stop_source_type = ::beman::lazy::detail::stop_source_of_t<C>;
    using stop_token_type  = decltype(std::declval<stop_source_type>().get_token());

    struct receiver {
        using receiver_concept = ::beman::execution::receiver_t;
        promise_type* self{};
        void set_value() && noexcept { std::coroutine_handle<promise_type>::from_promise(*this->self).resume(); }
        void set_error(auto&&) && noexcept {
            //-dk:TODO
        }
        void set_stopped() && noexcept {
            //-dk:TODO
        }
    };
    struct connector {
        decltype(::beman::execution::connect(
            ::beman::execution::schedule(::std::declval<::beman::lazy::detail::any_scheduler>()),
            ::std::declval<receiver>())) state;
        connector(::beman::lazy::detail::any_scheduler scheduler, receiver rcvr)
            : state(::beman::execution::connect(::beman::execution::schedule(::std::move(scheduler)),
                                                ::std::move(rcvr))) {}
    };

    void notify_complete() { this->state->complete(); }
    void start(auto&& e, ::beman::lazy::detail::state_base<C>* s) {
        this->state = s;
        if constexpr (std::same_as<::beman::lazy::detail::inline_scheduler, scheduler_type>) {
            this->scheduler.emplace();
            std::coroutine_handle<promise_type>::from_promise(*this).resume();
        } else {
            this->scheduler.emplace(::beman::execution::get_scheduler(e));
            this->initial_state.emplace(*this->scheduler, receiver{this});
            ::beman::execution::start(this->initial_state->state);
        }
    }

    template <typename... A>
    promise_type(const A&... a) : allocator(::beman::lazy::detail::find_allocator<allocator_type>(a...)) {}

    std::suspend_always initial_suspend() noexcept { return {}; }
    final_awaiter       final_suspend() noexcept { return {}; }
    void                unhandled_exception() { this->set_error(std::current_exception()); }
    auto                get_return_object() { return Coroutine(::beman::lazy::detail::handle<promise_type>(this)); }

    template <typename E>
    auto await_transform(::beman::lazy::detail::with_error<E> with) noexcept {
        // This overload is only used if error completions use `co_await with_error(e)`.
        return std::move(with);
    }
    template <::beman::execution::sender Sender>
    auto await_transform(Sender&& sender) noexcept {
        if constexpr (std::same_as<::beman::lazy::detail::inline_scheduler, scheduler_type>)
            return ::beman::execution::as_awaitable(std::forward<Sender>(sender), *this);
        else
            return ::beman::execution::as_awaitable(
                ::beman::execution::continues_on(std::forward<Sender>(sender), *(this->scheduler)), *this);
    }

    template <typename E>
    final_awaiter yield_value(with_error<E> with) noexcept {
        this->result.template emplace<E>(with.error);
        return {};
    }

    [[no_unique_address]] allocator_type  allocator;
    std::optional<scheduler_type>         scheduler{};
    ::beman::lazy::detail::state_base<C>* state{};
    ::std::optional<connector>            initial_state;

    std::coroutine_handle<> unhandled_stopped() {
        this->state->complete();
        return std::noop_coroutine();
    }

    struct env {
        const promise_type* promise;

        scheduler_type  query(::beman::execution::get_scheduler_t) const noexcept { return *promise->scheduler; }
        allocator_type  query(::beman::execution::get_allocator_t) const noexcept { return promise->allocator; }
        stop_token_type query(::beman::execution::get_stop_token_t) const noexcept {
            return promise->state->get_stop_token();
        }
        template <typename Q, typename... A>
            requires requires(const C& c, Q q, A&&... a) { q(c, std::forward<A>(a)...); }
        auto query(Q q, A&&... a) const noexcept {
            return q(promise->state->get_context(), std::forward<A>(a)...);
        }
    };

    env get_env() const noexcept { return {this}; }
};
} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
