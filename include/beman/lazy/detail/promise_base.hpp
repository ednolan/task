// include/beman/lazy/detail/promise_base.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_PROMISE_BASE
#define INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_PROMISE_BASE

#include <beman/lazy/detail/result_type.hpp>
#include <beman/execution/execution.hpp>
#include <cstddef>
#include <concepts>
#include <type_traits>
#include <variant>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {
/*
 * \brief Helper base class dealing with void vs. value results.
 * \headerfile beman/lazy/task.hpp <beman/lazy/task.hpp>
 * \internal
 */
template <::beman::lazy::detail::stoppable Stop, typename Value, typename ErrorCompletions>
class promise_base;

template <::beman::lazy::detail::stoppable Stop, typename Value, typename... Error>
    requires(not ::std::same_as<Value, void>)
class promise_base<Stop, Value, ::beman::execution::completion_signatures<::beman::execution::set_error_t(Error)...>>
    : public ::beman::lazy::detail::result_type<Stop, Value, Error...> {
  public:
    /*
     * \brief Set the value result.
     * \internal
     */
    template <typename T>
    void return_value(T&& value) {
        this->set_value(::std::forward<T>(value));
    }
};

template <typename ::beman::lazy::detail::stoppable Stop, typename... Error>
class promise_base<Stop, void, ::beman::execution::completion_signatures<::beman::execution::set_error_t(Error)...>>
    : public ::beman::lazy::detail::result_type<Stop, void_type, Error...> {
  public:
    /*
     * \brief Set the value result although without any value.
     */
    void return_void() { this->set_value(void_type{}); }
};
} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
