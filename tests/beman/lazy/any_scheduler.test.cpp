// tests/beman/lazy/any_scheduler.test.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/detail/any_scheduler.hpp>
#include <beman/lazy/detail/inline_scheduler.hpp>
#include <beman/execution26/execution.hpp>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

namespace ex = beman::execution26;
namespace ly = beman::lazy;

// ----------------------------------------------------------------------------

namespace {
struct thread_context {
    struct base {
        base* next{};
        virtual ~base()         = default;
        virtual void complete() = 0;
    };

    std::mutex              mutex;
    std::condition_variable condition;
    bool                    done{false};
    base*                   work{};
    std::thread             thread;

    base* get_work() {
        std::unique_lock cerberus(this->mutex);
        condition.wait(cerberus, [this] { return this->done || this->work; });
        base* rc{this->work};
        if (rc) {
            this->work = rc->next;
        }
        return rc;
    }
    void enqueue(base* w) {
        {
            std::lock_guard cerberus(this->mutex);
            w->next = std::exchange(this->work, w);
        }
        this->condition.notify_one();
    }

    thread_context()
        : thread([this] {
              while (auto w{this->get_work()}) {
                  w->complete();
              }
          }) {}
    ~thread_context() {
        this->stop();
        this->thread.join();
    }

    struct scheduler {
        using scheduler_concept = ex::scheduler_t;
        thread_context* context;
        bool            operator==(const scheduler&) const = default;

        template <ex::receiver Receiver>
        struct state : base {
            using operation_state_concept = ex::operation_state_t;

            thread_context*               ctxt;
            std::remove_cvref_t<Receiver> receiver;
            template <typename R>
            state(auto c, R&& r) : ctxt(c), receiver(std::forward<R>(r)) {}
            void start() & noexcept { this->ctxt->enqueue(this); }
            void complete() override { ex::set_value(std::move(this->receiver)); }
        };
        struct env {
            thread_context* ctxt;
            scheduler       query(const ex::get_completion_scheduler_t<ex::set_value_t>&) const noexcept {
                return scheduler{ctxt};
            }
        };
        struct sender {
            using sender_concept        = ex::sender_t;
            using completion_signatures = ex::completion_signatures<ex::set_value_t()>;

            thread_context* ctxt;

            template <ex::receiver Receiver>
            auto connect(Receiver&& receiver) {
                static_assert(ex::operation_state<state<Receiver>>);
                return state<Receiver>(this->ctxt, std::forward<Receiver>(receiver));
            }
            env get_env() const noexcept { return {this->ctxt}; }
        };
        static_assert(ex::sender<sender>);

        sender schedule() noexcept { return sender{this->context}; }
    };
    static_assert(ex::scheduler<scheduler>);

    scheduler get_scheduler() { return scheduler{this}; }
    void      stop() {
        {
            std::lock_guard cerberus(this->mutex);
            this->done = true;
        }
        this->condition.notify_one();
    }
};
} // namespace

// ----------------------------------------------------------------------------

int main() {
    static_assert(ex::scheduler<ly::detail::any_scheduler>);

    thread_context ctxt1;
    thread_context ctxt2;

    assert(ctxt1.get_scheduler() == ctxt1.get_scheduler());
    assert(ctxt2.get_scheduler() == ctxt2.get_scheduler());
    assert(ctxt1.get_scheduler() != ctxt2.get_scheduler());

    ly::detail::any_scheduler sched1(ctxt1.get_scheduler());
    ly::detail::any_scheduler sched2(ctxt2.get_scheduler());
    assert(sched1 == sched1);
    assert(sched2 == sched2);
    assert(sched1 != sched2);

    ly::detail::any_scheduler copy(sched1);
    assert(copy == sched1);
    assert(copy != sched2);
    ly::detail::any_scheduler move(std::move(copy));
    assert(move == sched1);
    assert(move != sched2);

    copy = sched2;
    assert(copy == sched2);
    assert(copy != sched1);

    move = std::move(copy);
    assert(move == sched2);
    assert(move != sched1);

    std::atomic<std::thread::id> id1{};
    std::atomic<std::thread::id> id2{};
    ex::sync_wait(ex::schedule(sched1) | ex::then([&id1]() { id1 = std::this_thread::get_id(); }));
    ex::sync_wait(ex::schedule(sched2) | ex::then([&id2]() { id2 = std::this_thread::get_id(); }));
    assert(id1 != id2);
    ex::sync_wait(ex::schedule(ly::detail::any_scheduler(sched1)) |
                  ex::then([&id1]() { assert(id1 == std::this_thread::get_id()); }));
    ex::sync_wait(ex::schedule(ly::detail::any_scheduler(sched2)) |
                  ex::then([&id2]() { assert(id2 == std::this_thread::get_id()); }));
}
