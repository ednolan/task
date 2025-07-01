// include/beman/task/detail/state_base.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_STATE_BASE
#define INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_STATE_BASE

#include <beman/task/detail/stop_source.hpp>

// ----------------------------------------------------------------------------

namespace beman::task::detail {
template <typename Context>
class state_base {
  public:
    using stop_source_type = ::beman::task::detail::stop_source_of_t<Context>;
    using stop_token_type  = decltype(std::declval<stop_source_type>().get_token());

    void            complete() { this->do_complete(); }
    stop_token_type get_stop_token() { return this->do_get_stop_token(); }
    Context&        get_context() { return this->do_get_context(); }

  protected:
    virtual void            do_complete()       = 0; // NOLINT(portability-template-virtual-member-function)
    virtual stop_token_type do_get_stop_token() = 0; // NOLINT(portability-template-virtual-member-function)
    virtual Context&        do_get_context()    = 0; // NOLINT(portability-template-virtual-member-function)

    virtual ~state_base() = default;
};
} // namespace beman::task::detail

// ----------------------------------------------------------------------------

#endif
