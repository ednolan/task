// include/beman/task/detail/into_optional.hpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_TASK
#define INCLUDED_BEMAN_LAZY_DETAIL_TASK

#include <beman/execution/execution.hpp>
#include <beman/task/detail/allocator_of.hpp>
#include <beman/task/detail/allocator_support.hpp>
#include <beman/task/detail/any_scheduler.hpp>
#include <beman/task/detail/completion.hpp>
#include <beman/task/detail/scheduler_of.hpp>
#include <beman/task/detail/stop_source.hpp>
#include <beman/task/detail/inline_scheduler.hpp>
#include <beman/task/detail/final_awaiter.hpp>
#include <beman/task/detail/handle.hpp>
#include <beman/task/detail/sub_visit.hpp>
#include <beman/task/detail/with_error.hpp>
#include <beman/task/detail/state_base.hpp>
#include <beman/task/detail/error_types_of.hpp>
#include <beman/task/detail/promise_type.hpp>
#include <concepts>
#include <coroutine>
#include <optional>
#include <type_traits>

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif

// ----------------------------------------------------------------------------

namespace beman::task::detail {

struct default_context {};

template <typename T = void, typename C = default_context>
class task {
  private:
    using stop_source_type = ::beman::task::detail::stop_source_of_t<C>;
    using stop_token_type  = decltype(std::declval<stop_source_type>().get_token());

  public:
    using sender_concept = ::beman::execution::sender_t;
    using completion_signatures =
        ::beman::execution::completion_signatures<beman::task::detail::completion_t<T>,
                                                  //-dk:TODO create the appropriate completion signatures
                                                  ::beman::execution::set_error_t(std::exception_ptr),
                                                  ::beman::execution::set_error_t(std::error_code),
                                                  ::beman::execution::set_stopped_t()>;

    using promise_type = ::beman::task::detail::promise_type<task, T, C>;

  private:
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
    struct state : ::beman::task::detail::state_base<C>, state_rep<Receiver> {
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

        ::beman::task::detail::handle<promise_type> handle;
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

    ::beman::task::detail::handle<promise_type> handle;

  private:
    friend promise_type;
    explicit task(::beman::task::detail::handle<promise_type> h) : handle(std::move(h)) {}

  public:
    task(const task&)                = delete;
    task(task&&) noexcept            = default;
    task& operator=(const task&)     = delete;
    task& operator=(task&&) noexcept = delete;
    ~task()                          = default;

    template <typename Receiver>
    state<Receiver> connect(Receiver receiver) {
        return state<Receiver>(std::forward<Receiver>(receiver), std::move(this->handle));
    }
};
} // namespace beman::task::detail

// ----------------------------------------------------------------------------

#endif
