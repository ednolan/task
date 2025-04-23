// examples/c++now-affinity.cpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>
#include <beman/execution/stop_token.hpp>
#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>

namespace ex = beman::execution;

// ----------------------------------------------------------------------------

class thread_context {
    struct base {
        virtual void do_run() = 0;
        base()                = default;
        base(base&&)          = delete;

      protected:
        ~base() = default;
    };

    ex::stop_source         source;
    std::mutex              mutex;
    std::condition_variable condition;
    std::thread             thread{[this] { this->run(this->source.get_token()); }};
    std::deque<base*>       queue;

    void run(auto token) {
        while (true) {
            base* next(std::invoke([&]() -> base* {
                std::unique_lock cerberus(this->mutex);
                this->condition.wait(cerberus,
                                     [this, &token] { return token.stop_requested() || not this->queue.empty(); });
                if (this->queue.empty()) {
                    return nullptr;
                }
                base* next = queue.front();
                queue.pop_front();
                return next;
            }));
            if (next) {
                next->do_run();
            } else {
                break;
            }
        }
    }

  public:
    thread_context()                 = default;
    thread_context(thread_context&&) = delete;
    ~thread_context() {
        this->source.request_stop();
        this->condition.notify_one();
        this->thread.join();
    }

    void enqueue(base* work) {
        {
            std::lock_guard cerberus(this->mutex);
            this->queue.push_back(work);
        }
        this->condition.notify_one();
    }

    template <typename Receiver>
    struct state final : base {
        using operation_state_concept = ex::operation_state_t;
        Receiver        receiver;
        thread_context* context;

        template <typename R>
        state(thread_context* ctxt, R&& r) : receiver(std::forward<R>(r)), context(ctxt) {}
        void start() noexcept {
            static_assert(ex::operation_state<state>);
            this->context->enqueue(this);
        }
        void do_run() override { ex::set_value(std::move(this->receiver)); }
    };

    struct scheduler;
    struct env {
        thread_context* context;
        template <typename Tag>
        scheduler query(const ex::get_completion_scheduler_t<Tag>&) const noexcept {
            return {this->context};
        }
    };
    struct sender {
        using sender_concept        = ex::sender_t;
        using completion_signatures = ex::completion_signatures<ex::set_value_t()>;
        thread_context* context;
        template <ex::receiver Receiver>
        auto connect(Receiver&& r) {
            return state<std::remove_cvref_t<Receiver>>(this->context, std::forward<Receiver>(r));
        }
        env get_env() const noexcept { return {this->context}; }
    };
    static_assert(ex::sender<sender>);
    // static_assert(ex::sender_in<sender>);

    struct scheduler {
        using scheduler_concept = ex::scheduler_t;
        thread_context* context;

        sender schedule() noexcept { return {this->context}; }
        bool   operator==(const scheduler&) const = default;
    };
    static_assert(ex::scheduler<scheduler>);
    scheduler get_scheduler() { return {this}; }
};

// ----------------------------------------------------------------------------

ex::task<> work1(auto sched) {
    std::cout << "before id=" << std::this_thread::get_id() << "\n";
    co_await (ex::schedule(sched) | ex::then([] { std::cout << "then id  =" << std::this_thread::get_id() << "\n"; }));
    std::cout << "after id =" << std::this_thread::get_id() << "\n\n";
}

ex::task<> work2(auto sched) {
    std::cout << "before id=" << std::this_thread::get_id() << "\n";
    auto s = co_await ex::change_coroutine_scheduler(ex::inline_scheduler());
    co_await (ex::schedule(sched) | ex::then([] { std::cout << "then id  =" << std::this_thread::get_id() << "\n"; }));
    std::cout << "after1 id=" << std::this_thread::get_id() << "\n";

    co_await ex::change_coroutine_scheduler(s);
    co_await (ex::schedule(sched) | ex::then([] { std::cout << "then id  =" << std::this_thread::get_id() << "\n"; }));
    std::cout << "after2 id=" << std::this_thread::get_id() << "\n\n";
}

struct inline_context {
    using scheduler_type = ex::inline_scheduler;
};

ex::task<void, inline_context> work3(auto sched) {
    std::cout << "before id=" << std::this_thread::get_id() << "\n";
    co_await (ex::schedule(sched) | ex::then([] { std::cout << "then id  =" << std::this_thread::get_id() << "\n"; }));
    std::cout << "after id =" << std::this_thread::get_id() << "\n\n";
}

int main() {
    std::cout << std::unitbuf;
    thread_context context;
    ex::sync_wait(work1(context.get_scheduler()));
    ex::sync_wait(work2(context.get_scheduler()));
    ex::sync_wait(work3(context.get_scheduler()));
}
