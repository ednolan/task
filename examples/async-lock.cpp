// examples/async-lock                                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <iostream>
#include <coroutine>
#include <exception>
#include <queue>
#include <mutex>
#include <string>
#include <condition_variable>
#include <beman/execution/execution.hpp>
#include <beman/task/task.hpp>

namespace ex = beman::execution;

struct queue {
    struct notify {
        virtual ~notify()          = default;
        virtual void complete(int) = 0;
    };

    std::mutex                           mutex;
    std::condition_variable              condition;
    std::queue<std::tuple<notify*, int>> work;

    void push(notify* n, int w) {
        {
            std::lock_guard cerberos{this->mutex};
            work.emplace(n, w);
        }
        condition.notify_one();
    }
    std::tuple<notify*, int> pop() {
        std::unique_lock cerberos{this->mutex};
        condition.wait(cerberos, [this] { return not work.empty(); });
        auto result{work.front()};
        work.pop();
        return result;
    }
};

struct request {
    using sender_concept        = ex::sender_t;
    using completion_signatures = ex::completion_signatures<ex::set_value_t(int)>;

    template <typename Receiver>
    struct state : queue::notify {
        using operation_state_concept = ex::operation_state_t;
        Receiver receiver;
        int      value;
        queue&   que;

        template <typename R>
        state(R&& r, int val, queue& q) : receiver(std::forward<R>(r)), value(val), que(q) {}

        void start() & noexcept { this->que.push(this, value); }
        void complete(int result) { ex::set_value(std::move(this->receiver), result); }
    };

    int    value;
    queue& que;

    template <typename Receiver>
    auto connect(Receiver&& receiver) {
        return state<std::remove_cvref_t<Receiver>>(std::forward<Receiver>(receiver), this->value, this->que);
    }
};

void stop(queue& q) { ex::sync_wait(request{0, q}); }

int main(int ac, char* av[]) {
    try {
    queue q{};

    std::thread process([&q] {
        while (true) {
            auto [completion, request] = q.pop();
            completion->complete(2 * request);
            if (request == 0) {
                break;
            }
        }
    });

    auto work{[](queue& que) -> ex::task<void> {
        std::cout << std::this_thread::get_id() << " start\n" << std::flush;
        auto result = co_await request{17, que};
        std::cout << std::this_thread::get_id() << " result=" << result << "\n" << std::flush;
        stop(que);
    }};

    if (1 < ac && av[1] == std::string_view("run-it")) {
        ex::sync_wait(ex::detail::write_env(work(q), ex::detail::make_env(ex::get_scheduler, ex::inline_scheduler{})));
    } else {
        ex::sync_wait(work(q));
    }
    process.join();
    std::cout << "done\n";
    }
    catch(...) {
        std::cout << "ERROR: unexpected exception\n";
    }
}
