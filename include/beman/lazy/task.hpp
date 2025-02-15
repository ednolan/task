// include/beman/lazy/task.hpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_LAZY_TASK
#define INCLUDED_INCLUDE_BEMAN_LAZY_TASK

#include <beman/lazy/detail/allocator_of.hpp>
#include <beman/lazy/detail/any_scheduler.hpp>
#include <beman/lazy/detail/inline_scheduler.hpp>
#include <beman/lazy/detail/into_optional.hpp>
#include <beman/lazy/detail/task.hpp>
#include <beman/lazy/detail/scheduler_of.hpp>
#include <beman/lazy/detail/stop_source.hpp>

// ----------------------------------------------------------------------------

namespace beman::lazy {
template <typename Context>
using allocator_of_t = ::beman::lazy::detail::allocator_of_t<Context>;
template <typename Context>
using scheduler_of_t = ::beman::lazy::detail::scheduler_of_t<Context>;
template <typename Context>
using stop_source_of_t = ::beman::lazy::detail::stop_source_of_t<Context>;

using any_scheduler    = ::beman::lazy::detail::any_scheduler;
using inline_scheduler = ::beman::lazy::detail::inline_scheduler;
using into_optional_t  = ::beman::lazy::detail::into_optional_t;
using ::beman::lazy::detail::into_optional;

using ::beman::lazy::detail::default_context;
using ::beman::lazy::detail::with_error;
template <typename T = void, typename Context = ::beman::lazy::default_context>
using task = ::beman::lazy::detail::task<T, Context>;
} // namespace beman::lazy

namespace beman::execution {
template <typename Context>
using allocator_of_t = ::beman::lazy::detail::allocator_of_t<Context>;
template <typename Context>
using scheduler_of_t = ::beman::lazy::detail::scheduler_of_t<Context>;
template <typename Context>
using stop_source_of_t = ::beman::lazy::detail::stop_source_of_t<Context>;

using any_scheduler    = ::beman::lazy::detail::any_scheduler;
using inline_scheduler = ::beman::lazy::detail::inline_scheduler;
using into_optional_t  = ::beman::lazy::detail::into_optional_t;
using ::beman::lazy::detail::into_optional;

using ::beman::lazy::detail::default_context;
using ::beman::lazy::detail::with_error;
template <typename T = void, typename Context = ::beman::lazy::default_context>
using task = ::beman::lazy::detail::task<T, Context>;

template <typename T = void, typename Context = ::beman::lazy::default_context>
using lazy [[deprecated("beman::execution::lazy has been renamed to beman::execution::task")]] =
    ::beman::lazy::task<T, Context>;
} // namespace beman::execution

// ----------------------------------------------------------------------------

#endif
