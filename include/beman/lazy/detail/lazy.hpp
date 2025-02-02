// include/beman/lazy/detail/into_optional.hpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_LAZY
#define INCLUDED_BEMAN_LAZY_DETAIL_LAZY

#include <beman/execution/execution.hpp>
#include <beman/lazy/detail/allocator_of.hpp>
#include <beman/lazy/detail/allocator_support.hpp>
#include <beman/lazy/detail/any_scheduler.hpp>
#include <beman/lazy/detail/completion.hpp>
#include <beman/lazy/detail/scheduler_of.hpp>
#include <beman/lazy/detail/stop_source.hpp>
#include <beman/lazy/detail/promise_base.hpp>
#include <beman/lazy/detail/inline_scheduler.hpp>
#include <beman/lazy/detail/final_awaiter.hpp>
#include <beman/lazy/detail/handle.hpp>
#include <beman/lazy/detail/sub_visit.hpp>
#include <beman/lazy/detail/with_error.hpp>
#include <beman/lazy/detail/state_base.hpp>
#include <beman/lazy/detail/error_types_of.hpp>
#include <concepts>
#include <coroutine>
#include <optional>
#include <type_traits>

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {

struct default_context {};

template <typename T = void, typename C = default_context>
struct lazy {
    using allocator_type   = ::beman::lazy::detail::allocator_of_t<C>;
    using scheduler_type   = ::beman::lazy::detail::scheduler_of_t<C>;
    using stop_source_type = ::beman::lazy::detail::stop_source_of_t<C>;
    using stop_token_type  = decltype(std::declval<stop_source_type>().get_token());

    using sender_concept = ::beman::execution::sender_t;
    using completion_signatures =
        ::beman::execution::completion_signatures<beman::lazy::detail::completion_t<T>,
                                                  //-dk:TODO create the appropriate completion signatures
                                                  ::beman::execution::set_error_t(std::exception_ptr),
                                                  ::beman::execution::set_error_t(std::error_code),
                                                  ::beman::execution::set_stopped_t()>;

    struct promise_type : ::beman::lazy::detail::promise_base<::beman::lazy::detail::stoppable::yes,
                                                              ::std::remove_cvref_t<T>,
                                                              ::beman::lazy::detail::error_types_of_t<C>>,
                          ::beman::lazy::detail::allocator_support<allocator_type> {
        void notify_complete() { this->state->complete(); }
        void start(auto&& e, ::beman::lazy::detail::state_base<C>* s) {
            this->state = s;
            if constexpr (std::same_as<::beman::lazy::detail::inline_scheduler, scheduler_type>)
                this->scheduler.emplace();
            else
                this->scheduler.emplace(::beman::execution::get_scheduler(e));
            std::coroutine_handle<promise_type>::from_promise(*this).resume();
        }

        template <typename... A>
        promise_type(const A&... a) : allocator(::beman::lazy::detail::find_allocator<allocator_type>(a...)) {}

        std::suspend_always initial_suspend() noexcept { return {}; /*-dk:TODO resume on the correct scheduler */ }
        final_awaiter       final_suspend() noexcept { return {}; }
        void                unhandled_exception() { this->set_error(std::current_exception()); }
        auto                get_return_object() { return lazy(::beman::lazy::detail::handle<promise_type>(this)); }

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

        [[no_unique_address]] allocator_type allocator;
        std::optional<scheduler_type>        scheduler{};
        ::beman::lazy::detail::state_base<C>* state{};

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

    template <typename Receiver>
    struct state_rep {
        std::remove_cvref_t<Receiver> receiver;
        C                             context;
        template <typename R>
        state_rep(R&& r) : receiver(std::forward<R>(r)), context() {}
    };
    template <typename Receiver>
        requires requires { C(::beman::execution::get_env(std::declval<std::remove_cvref_t<Receiver>&>())); } &&
                 (not requires(const Receiver& receiver) {
                     typename C::template env_type<decltype(::beman::execution::get_env(receiver))>;
                 })
    struct state_rep<Receiver> {
        std::remove_cvref_t<Receiver> receiver;
        C                             context;
        template <typename R>
        state_rep(R&& r) : receiver(std::forward<R>(r)), context(::beman::execution::get_env(this->receiver)) {}
    };
    template <typename Receiver>
        requires requires(const Receiver& receiver) {
            typename C::template env_type<decltype(::beman::execution::get_env(receiver))>;
        }
    struct state_rep<Receiver> {
        using upstream_env = decltype(::beman::execution::get_env(std::declval<std::remove_cvref_t<Receiver>&>()));
        std::remove_cvref_t<Receiver>               receiver;
        typename C::template env_type<upstream_env> own_env;
        C                                           context;
        template <typename R>
        state_rep(R&& r)
            : receiver(std::forward<R>(r)),
              own_env(::beman::execution::get_env(this->receiver)),
              context(this->own_env) {}
    };

    template <typename Receiver>
    struct state : ::beman::lazy::detail::state_base<C>, state_rep<Receiver> {
        using operation_state_concept = ::beman::execution::operation_state_t;
        using stop_token_t =
            decltype(::beman::execution::get_stop_token(::beman::execution::get_env(std::declval<Receiver>())));
        struct stop_link {
            stop_source_type& source;
            void              operator()() const noexcept { source.request_stop(); }
        };
        using stop_callback_t = ::beman::execution::stop_callback_for_t<stop_token_t, stop_link>;
        template <typename R, typename H>
        state(R&& r, H h) : state_rep<Receiver>(std::forward<R>(r)), handle(std::move(h)) {}

        ::beman::lazy::detail::handle<promise_type> handle;
        stop_source_type                            source;
        std::optional<stop_callback_t>              stop_callback;

        void            start() & noexcept { this->handle.start(::beman::execution::get_env(this->receiver), this); }
        void            do_complete() override { this->handle.complete(::std::move(this->receiver)); }
        stop_token_type do_get_stop_token() override {
            if (this->source.stop_possible() && not this->stop_callback) {
                this->stop_callback.emplace(
                    ::beman::execution::get_stop_token(::beman::execution::get_env(this->receiver)),
                    stop_link{this->source});
            }
            return this->source.get_token();
        }
        C& do_get_context() override { return this->context; }
    };

    ::beman::lazy::detail::handle<promise_type> handle;

  private:
    explicit lazy(::beman::lazy::detail::handle<promise_type> h) : handle(std::move(h)) {}

  public:
    template <typename Receiver>
    state<Receiver> connect(Receiver receiver) {
        return state<Receiver>(std::forward<Receiver>(receiver), std::move(this->handle));
    }
};
} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
