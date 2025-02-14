// include/beman/lazy/detail/scheduler_of.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_SCHEDULER_OF
#define INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_SCHEDULER_OF

#include <beman/lazy/detail/any_scheduler.hpp>
#include <beman/execution/execution.hpp>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {
/*!
 * \brief Utility to get a scheduler type from a context
 * \headerfile beman/lazy/task.hpp <beman/lazy/task.hpp>
 * \internal
 */
template <typename>
struct scheduler_of {
    using type = ::beman::lazy::detail::any_scheduler;
};
template <typename Context>
    requires requires { typename Context::scheduler_type; }
struct scheduler_of<Context> {
    using type = typename Context::scheduler_type;
    static_assert(::beman::execution::scheduler<type>, "The type alias scheduler_type needs to refer to a scheduler");
};
template <typename Context>
using scheduler_of_t = typename scheduler_of<Context>::type;
} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
