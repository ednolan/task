// include/beman/task/detail/promise_type.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_PROMISE_TYPE
#define INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_PROMISE_TYPE

#include <beman/task/detail/awaiter.hpp>
#include <beman/task/detail/affine_on.hpp>
#include <beman/task/detail/allocator_of.hpp>
#include <beman/task/detail/allocator_support.hpp>
#include <beman/task/detail/change_coroutine_scheduler.hpp>
#include <beman/task/detail/error_types_of.hpp>
#include <beman/task/detail/final_awaiter.hpp>
#include <beman/task/detail/find_allocator.hpp>
#include <beman/task/detail/handle.hpp>
#include <beman/task/detail/inline_scheduler.hpp>
#include <beman/task/detail/promise_base.hpp>
#include <beman/task/detail/result_type.hpp>
#include <beman/task/detail/scheduler_of.hpp>
#include <beman/task/detail/state_base.hpp>
#include <beman/task/detail/with_error.hpp>
#include <beman/execution/execution.hpp>
#include <beman/execution/detail/meta_contains.hpp>
#include <beman/task/detail/promise_env.hpp>
#include <cassert>
#include <coroutine>
#include <optional>
#include <type_traits>
#include <beman/task/detail/logger.hpp>

// ----------------------------------------------------------------------------

namespace beman::task::detail {

#if 0
template <typename T>
struct has_exception_ptr;
template <typename... T>
struct has_exception_ptr<::beman::execution::completion_signatures<T...>> {
    static constexpr bool value{
        ::beman::execution::detail::meta::contains<::beman::execution::set_error_t(::std::exception_ptr), T...>};
};

template <typename T>
inline constexpr bool has_exception_ptr_v{::beman::task::detail::has_exception_ptr<T>::value};

template <typename Coroutine, typename T, typename C>
struct promise_type : ::beman::task::detail::promise_base<::beman::task::detail::stoppable::yes,
                                                          ::std::remove_cvref_t<T>,
                                                          ::beman::task::detail::error_types_of_t<C>>,
                      ::beman::task::detail::allocator_support<::beman::task::detail::allocator_of_t<C>> {

    void start([[maybe_unused]] auto&& e, ::beman::task::detail::state_base<C>* s) {
        this->state = s;
        if constexpr (std::same_as<::beman::task::detail::inline_scheduler, scheduler_type>) {
            this->scheduler.emplace();
        } else {
            this->scheduler.emplace(::beman::execution::get_scheduler(e));
        }
        this->initial->run();
    }

    struct initial_base {
        virtual ~initial_base() = default;
        virtual void run()      = 0;
    };
    struct initial_sender {
        using sender_concept        = ::beman::execution::sender_t;
        using completion_signatures = ::beman::execution::completion_signatures<::beman::execution::set_value_t()>;

        template <::beman::execution::receiver Receiver>
        struct state : initial_base {
            using operation_state_concept = ::beman::execution::operation_state_t;
            promise_type* promise;
            Receiver      receiver;
            template <typename R>
            state(promise_type* p, R&& r) : promise(p), receiver(::std::forward<R>(r)) {}
            void start() & noexcept { this->promise->initial = this; }
            void run() override { ::beman::execution::set_value(::std::move(receiver)); }
        };

        promise_type* promise{};
        template <::beman::execution::receiver Receiver>
        auto connect(Receiver&& receiver) {
            return state<::std::remove_cvref_t<Receiver>>(this->promise, ::std::forward<Receiver>(receiver));
        }
    };

    auto initial_suspend() noexcept {
        return this->internal_await_transform(initial_sender{this},
                                              optional_ref_scheduler<scheduler_type>{&this->scheduler});
    }
    void          unhandled_exception() {
        if constexpr (::beman::task::detail::has_exception_ptr_v<::beman::task::detail::error_types_of_t<C>>) {
            this->set_error(std::current_exception());
        } else {
            std::terminate();
        }
    }
    auto get_return_object() noexcept { return Coroutine(::beman::task::detail::handle<promise_type>(this)); }

    template <typename E>
    auto await_transform(::beman::task::detail::with_error<E> with) noexcept {
        // This overload is only used if error completions use `co_await with_error(e)`.
        return std::move(with);
    }
    template <typename Env, typename P>
    auto await_transform(::beman::task::detail::awaiter<Env, P>&& a) noexcept {
        return ::std::move(a);
    }
    template <::beman::execution::sender Sender, typename Scheduler>
    auto internal_await_transform(Sender&& sender, Scheduler&& sched) noexcept {
        if constexpr (std::same_as<::beman::task::detail::inline_scheduler, scheduler_type>)
            return ::beman::execution::as_awaitable(std::forward<Sender>(sender), *this);
        else
            return ::beman::execution::as_awaitable(
                ::beman::task::affine_on(::std::forward<Sender>(sender), ::std::forward<Scheduler>(sched)), *this);
    }
    template <::beman::execution::sender Sender>
    auto await_transform(Sender&& sender) noexcept {
        return this->internal_await_transform(::std::forward<Sender>(sender), *this->scheduler);
    }
    auto await_transform(::beman::task::detail::change_coroutine_scheduler<scheduler_type> c) {
        return ::std::move(c);
    }

    [[no_unique_address]] allocator_type  allocator;
    std::optional<scheduler_type>         scheduler{};
    ::beman::task::detail::state_base<C>* state{};
    initial_base*                         initial{};

    std::coroutine_handle<> unhandled_stopped() {
        this->state->complete();
        return std::noop_coroutine();
    }


    auto get_env() const noexcept -> env_t { return env_t{this}; }
};
#else

template <typename Coroutine, typename T, typename Environment>
class promise_type
    : public ::beman::task::detail::promise_base<::beman::task::detail::stoppable::yes,
                                                 ::std::remove_cvref_t<T>,
                                                 ::beman::task::detail::error_types_of_t<Environment>>,
      public ::beman::task::detail::allocator_support<::beman::task::detail::allocator_of_t<Environment>> {
  public:
    using allocator_type   = ::beman::task::detail::allocator_of_t<Environment>;
    using scheduler_type   = ::beman::task::detail::scheduler_of_t<Environment>;
    using stop_source_type = ::beman::task::detail::stop_source_of_t<Environment>;
    using stop_token_type  = decltype(std::declval<stop_source_type>().get_token());

    template <typename... A>
    promise_type(const A&... a) : allocator(::beman::task::detail::find_allocator<allocator_type>(a...)) {}

    constexpr auto initial_suspend() noexcept -> ::std::suspend_always {
        ::beman::task::detail::logger l("promise_type::initial_suspend");
        return {};
    }
    constexpr auto final_suspend() noexcept -> ::beman::task::detail::final_awaiter {
        ::beman::task::detail::logger l("promise_type::final_suspend");
        return {};
    }

    auto                    unhandled_exception() noexcept { /*-dk:TODO*/ }
    std::coroutine_handle<> unhandled_stopped() {
        this->state->complete();
        return std::noop_coroutine();
    }

    auto get_return_object() noexcept { return Coroutine(::beman::task::detail::handle<promise_type>(this)); }

    template <::beman::execution::sender Sender>
    auto await_transform(Sender&& sender) noexcept {
        ::beman::task::detail::logger l("promise_type::await_transform(sender)");
        if constexpr (requires { ::std::forward<Sender>(sender).as_awaitable(); }) {
            l.log("using sender.as_awaitable");
            return ::std::forward<Sender>(sender).as_awaitable();
        } else if constexpr (requires { ::beman::execution::as_awaitable(::std::forward<Sender>(sender), *this); }) {
            l.log("using as_awaitable");
            return ::beman::execution::as_awaitable(::std::forward<Sender>(sender), *this);
        } else {
            l.log("using as_awaitable(affine_one(...))");
            return ::beman::execution::as_awaitable(
                ::beman::task::affine_on(::std::forward<Sender>(sender), this->get_scheduler()), *this);
        }
    }
    auto await_transform(::beman::task::detail::change_coroutine_scheduler<scheduler_type> c) {
        return ::std::move(c);
    }

    template <typename E>
    auto yield_value(with_error<E> with) noexcept -> ::beman::task::detail::final_awaiter {
        this->set_error(::std::move(with.error));
        return {};
    }

    auto get_env() const noexcept -> ::beman::task::detail::promise_env<promise_type> { return {this}; }

    auto start(::beman::task::detail::state_base<Environment>* state) -> ::std::coroutine_handle<> {
        ::beman::task::detail::logger l("promise_type::start");
        this->state = state;
        return ::std::coroutine_handle<promise_type>::from_promise(*this);
    }
    auto           notify_complete() -> ::std::coroutine_handle<> { return this->state->complete(); }
    scheduler_type change_scheduler(scheduler_type other) { return this->state->set_scheduler(::std::move(other)); }

    auto get_scheduler() const noexcept -> scheduler_type { return this->state->get_scheduler(); }
    auto get_allocator() const noexcept -> allocator_type { return this->allocator; }
    auto get_stop_token() const noexcept -> stop_token_type { return this->state->get_stop_token(); }
    auto get_context() const noexcept -> const Environment& { return this->state->get_context(); }

  private:
    allocator_type                                  allocator{};
    ::std::optional<scheduler_type>                 scheduler{};
    ::beman::task::detail::state_base<Environment>* state{};
};
#endif
} // namespace beman::task::detail

// ----------------------------------------------------------------------------

#endif
