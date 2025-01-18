// include/beman/lazy/detail/scheduler_of.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_SCHEDULER_OF
#define INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_SCHEDULER_OF

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {
template <typename>
struct scheduler_of {
    using type = ::beman::lazy::detail::any_scheduler;
};
template <typename Context>
    requires requires { typename Context::scheduler_type; }
struct scheduler_of<Context> {
    using type = typename Context::scheduler_type;
};
template <typename Context>
using scheduler_of_t = typename scheduler_of<Context>::type;
}

// ----------------------------------------------------------------------------

#endif
