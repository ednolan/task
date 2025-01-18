// include/beman/lazy/detail/any_scheduler.hpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_ANY_SCHEDULER
#define INCLUDED_BEMAN_LAZY_DETAIL_ANY_SCHEDULER

#include <beman/execution26/execution.hpp>
#include <beman/lazy/detail/poly.hpp>
#include <new>
#include <utility>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {

class any_scheduler {
    // TODO: add support for forwarding stop_tokens to the type-erased sender
    // TODO: other errors than std::exception_ptr should be supported
    struct state_base {
        virtual ~state_base()                           = default;
        virtual void complete_value()                   = 0;
        virtual void complete_error(std::exception_ptr) = 0;
        virtual void complete_stopped()                 = 0;
    };

    struct inner_state {
        struct receiver {
            using receiver_concept = ::beman::execution26::receiver_t;
            state_base* state;
            void        set_value() && noexcept { this->state->complete_value(); }
            void        set_error(std::exception_ptr ptr) && noexcept { this->state->complete_error(std::move(ptr)); }
            template <typename E>
            void set_error(E e) {
                try {
                    throw std::move(e);
                } catch (...) {
                    this->state->complete_error(std::current_exception());
                }
            }
            void set_stopped() && noexcept { this->state->complete_stopped(); }
        };
        static_assert(::beman::execution26::receiver<receiver>);

        struct base {
            virtual ~base()      = default;
            virtual void start() = 0;
        };
        template <::beman::execution26::sender Sender>
        struct concrete : base {
            using state_t = decltype(::beman::execution26::connect(std::declval<Sender>(), std::declval<receiver>()));
            state_t state;
            template <::beman::execution26::sender S>
            concrete(S&& s, state_base* b) : state(::beman::execution26::connect(std::forward<S>(s), receiver{b})) {}
            void start() override { ::beman::execution26::start(state); }
        };
        ::beman::lazy::detail::poly<base, 8u * sizeof(void*)> state;
        template <::beman::execution26::sender S>
        inner_state(S&& s, state_base* b) : state(static_cast<concrete<S>*>(nullptr), std::forward<S>(s), b) {}
        void start() { this->state->start(); }
    };

    template <::beman::execution26::receiver Receiver>
    struct state : state_base {
        using operation_state_concept = ::beman::execution26::operation_state_t;

        std::remove_cvref_t<Receiver> receiver;
        inner_state                   s;
        template <::beman::execution26::receiver R, typename PS>
        state(R&& r, PS& ps) : receiver(std::forward<R>(r)), s(ps->connect(this)) {}
        void start() & noexcept { this->s.start(); }
        void complete_value() override { ::beman::execution26::set_value(std::move(this->receiver)); }
        void complete_error(std::exception_ptr ptr) override {
            ::beman::execution26::set_error(std::move(receiver), std::move(ptr));
        }
        void complete_stopped() override { ::beman::execution26::set_stopped(std::move(this->receiver)); }
    };

    class sender;
    class env {
        friend class sender;

      private:
        const sender* sndr;
        env(const sender* s) : sndr(s) {}

      public:
        any_scheduler query(const ::beman::execution26::get_completion_scheduler_t<::beman::execution26::set_value_t>&)
            const noexcept {
            return this->sndr->inner_sender->get_completion_scheduler();
        }
    };

    // sender implementation
    class sender {
        friend class env;

      private:
        struct base {
            virtual ~base()                          = default;
            virtual base*       move(void*)          = 0;
            virtual base*       clone(void*) const   = 0;
            virtual inner_state connect(state_base*) = 0;
            virtual any_scheduler get_completion_scheduler() const = 0;
        };
        template <::beman::execution26::scheduler Scheduler>
        struct concrete : base {
            using sender_t = decltype(::beman::execution26::schedule(std::declval<Scheduler>()));
            sender_t sender;

            template <::beman::execution26::scheduler S>
            concrete(S&& s) : sender(::beman::execution26::schedule(std::forward<S>(s))) {}
            base*       move(void* buffer) override { return new (buffer) concrete(std::move(*this)); }
            base*       clone(void* buffer) const override { return new (buffer) concrete(*this); }
            inner_state connect(state_base* b) override { return inner_state(::std::move(sender), b); }
            any_scheduler get_completion_scheduler() const override {
                return any_scheduler(::beman::execution26::get_completion_scheduler<::beman::execution26::set_value_t>(
                    ::beman::execution26::get_env(this->sender)));
            }
        };
        poly<base, 4 * sizeof(void*)> inner_sender;

      public:
        using sender_concept = ::beman::execution26::sender_t;
        using completion_signatures =
            ::beman::execution26::completion_signatures<::beman::execution26::set_value_t(),
                                                        ::beman::execution26::set_error_t(std::exception_ptr),
                                                        ::beman::execution26::set_stopped_t()>;

        template <::beman::execution26::scheduler S>
        explicit sender(S&& s) : inner_sender(static_cast<concrete<S>*>(nullptr), std::forward<S>(s)) {}
        sender(sender&&)      = default;
        sender(const sender&) = default;

        template <::beman::execution26::receiver R>
        state<R> connect(R&& r) {
            return state<R>(std::forward<R>(r), this->inner_sender);
        }

        env get_env() const noexcept { return env(this); }
    };

    // scheduler implementation
    struct base {
        virtual ~base()                          = default;
        virtual sender schedule()                = 0;
        virtual base*  move(void* buffer)        = 0;
        virtual base*  clone(void*) const        = 0;
        virtual bool   equals(const base*) const = 0;
    };
    template <::beman::execution26::scheduler Scheduler>
    struct concrete : base {
        Scheduler scheduler;
        template <::beman::execution26::scheduler S>
        explicit concrete(S&& s) : scheduler(std::forward<S>(s)) {}
        sender schedule() override { return sender(this->scheduler); }
        base*  move(void* buffer) override { return new (buffer) concrete(std::move(*this)); }
        base*  clone(void* buffer) const override { return new (buffer) concrete(*this); }
        bool   equals(const base* o) const override {
            auto other{dynamic_cast<const concrete*>(o)};
            return other ? this->scheduler == other->scheduler : false;
        }
    };

    poly<base, 4 * sizeof(void*)> scheduler;

  public:
    using scheduler_concept = ::beman::execution26::scheduler_t;

    template <typename S>
        requires(not std::same_as<any_scheduler, std::remove_cvref_t<S>>)
    explicit any_scheduler(S&& s) : scheduler(static_cast<concrete<std::decay_t<S>>*>(nullptr), std::forward<S>(s)) {}

    sender schedule() { return this->scheduler->schedule(); }
    bool operator==(const any_scheduler&) const = default;
};
static_assert(::beman::execution26::scheduler<any_scheduler>);

} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
