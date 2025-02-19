// include/beman/task/detail/affine_on.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_AFFINE_ON
#define INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_AFFINE_ON

#include <beman/execution/execution.hpp>
#include <utility>

// ----------------------------------------------------------------------------

namespace beman::task::detail {
    struct affine_on_t {
        template <::beman::execution::sender Sender, ::beman::execution::scheduler Scheduler>
        auto operator()(Sender&& sender, Scheduler&& scheduler) const {
            return ::beman::execution::continues_on(::std::forward<Sender>(sender), ::std::forward<Scheduler>(scheduler));
        }
    };
}

namespace beman::task {
    inline constexpr ::beman::task::detail::affine_on_t affine_on{};
}

// ----------------------------------------------------------------------------

#endif
