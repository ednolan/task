// include/beman/lazy/detail/inline_scheduler.hpp                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_INLINE_SCHEDULER
#define INCLUDED_BEMAN_LAZY_DETAIL_INLINE_SCHEDULER

#include <beman/execution26/execution.hpp>
#include <utility>
#include <type_traits>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {
    struct inline_scheduler {
        struct env {
            inline_scheduler query(::beman::execution26::get_completion_scheduler_t<::beman::execution26::set_value_t> const&) const noexcept {
                return {};
            }
        };
        template <::beman::execution26::receiver Receiver>
        struct state {
            using operation_state_concept = ::beman::execution26::operation_state_t;
            std::remove_cvref_t<Receiver> receiver;
            void start() & noexcept {
                ::beman::execution26::set_value(std::move(receiver));
            }
        };
        struct sender {
            using sender_concept = ::beman::execution26::sender_t;
            using completion_signatures = ::beman::execution26::completion_signatures<::beman::execution26::set_value_t()>;

            env get_env() const noexcept { return {}; }
            template <::beman::execution26::receiver Receiver>
            state<Receiver> connect(Receiver&& receiver) { return { std::forward<Receiver>(receiver)}; }
        };
        static_assert(::beman::execution26::sender<sender>);

        using scheduler_concept = ::beman::execution26::scheduler_t;
        inline_scheduler() = default;
        template <typename Scheduler>
        explicit inline_scheduler(Scheduler&&) { static_assert(::beman::execution26::scheduler<Scheduler>); }
        sender schedule() noexcept { return {}; }
        bool operator== (inline_scheduler const&) const = default;
    };
    static_assert(::beman::execution26::scheduler<inline_scheduler>);
}

// ----------------------------------------------------------------------------

#endif
