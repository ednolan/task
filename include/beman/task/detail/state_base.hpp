// include/beman/task/detail/state_base.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_STATE_BASE
#define INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_STATE_BASE

#include <beman/task/detail/stop_source.hpp>
#include <beman/task/detail/scheduler_of.hpp>
#include <beman/task/detail/error_types_of.hpp>
#include <beman/task/detail/result_type.hpp>
#include <beman/task/detail/logger.hpp>
#include <coroutine>

// ----------------------------------------------------------------------------

namespace beman::task::detail {
template <typename Value, typename Environment>
class state_base : public ::beman::task::detail::result_type<::beman::task::detail::stoppable::yes,
                                                             Value,
                                                             ::beman::task::detail::error_types_of_t<Environment>> {
  public:
    using stop_source_type = ::beman::task::detail::stop_source_of_t<Environment>;
    using stop_token_type  = decltype(std::declval<stop_source_type>().get_token());
    using scheduler_type   = ::beman::task::detail::scheduler_of_t<Environment>;

    std::coroutine_handle<> complete() { return this->do_complete(); }
    stop_token_type get_stop_token() { return this->do_get_stop_token(); }
    Environment&            get_environment() { return this->do_get_environment(); }
    scheduler_type          get_scheduler() { return this->do_get_scheduler(); }
    scheduler_type          set_scheduler(scheduler_type other) { return this->do_set_scheduler(other); }

  protected:
    template <::beman::execution::scheduler Scheduler, typename Env>
    static auto from_env(const Env& env) {
        ::beman::task::detail::logger l{"state_base::from_env"};
        if constexpr (requires { Scheduler(::beman::execution::get_scheduler(env)); }) {
            l.log("found scheduler");
            return Scheduler(::beman::execution::get_scheduler(env));
        } else {
            l.log("no scheduler");
            return Scheduler();
        }
    }

    virtual std::coroutine_handle<> do_complete()        = 0; // NOLINT(portability-template-virtual-member-function)
    virtual stop_token_type do_get_stop_token() = 0; // NOLINT(portability-template-virtual-member-function)
    virtual Environment&            do_get_environment() = 0; // NOLINT(portability-template-virtual-member-function)
    virtual scheduler_type          do_get_scheduler()   = 0; // NOLINT(portability-template-virtual-member-function)
    virtual scheduler_type
    do_set_scheduler(scheduler_type other) = 0; // NOLINT(portability-template-virtual-member-function)

    virtual ~state_base() = default;
};
} // namespace beman::task::detail

// ----------------------------------------------------------------------------

#endif
