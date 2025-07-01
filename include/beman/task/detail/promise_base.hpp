// include/beman/task/detail/promise_base.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_PROMISE_BASE
#define INCLUDED_INCLUDE_BEMAN_TASK_DETAIL_PROMISE_BASE

#include <beman/task/detail/result_type.hpp>
#include <beman/execution/execution.hpp>
#include <cstddef>
#include <concepts>
#include <type_traits>
#include <variant>

// ----------------------------------------------------------------------------

namespace beman::task::detail {
/*
 * \brief Helper base class dealing with void vs. value results.
 * \headerfile beman/task/task.hpp <beman/task/task.hpp>
 * \internal
 */
template <::beman::task::detail::stoppable Stop, typename Value, typename ErrorCompletions>
class promise_base;

template <::beman::task::detail::stoppable Stop, typename Value>
    requires(not ::std::same_as<Value, void>)
class promise_base<Stop, Value, ::beman::execution::completion_signatures<>>
    : public ::beman::task::detail::result_type<Stop, Value> {
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

template <::beman::task::detail::stoppable Stop, typename Value, typename... Error>
    requires(not ::std::same_as<Value, void>)
class promise_base<Stop, Value, ::beman::execution::completion_signatures<::beman::execution::set_error_t(Error)...>>
    : public ::beman::task::detail::result_type<Stop, Value, Error...> {
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

template <typename ::beman::task::detail::stoppable Stop>
class promise_base<Stop, void, ::beman::execution::completion_signatures<>>
    : public ::beman::task::detail::result_type<Stop, void_type> {
  public:
    /*
     * \brief Set the value result although without any value.
     */
    void return_void() { this->set_value(void_type{}); }
};

template <typename ::beman::task::detail::stoppable Stop, typename... Error>
class promise_base<Stop, void, ::beman::execution::completion_signatures<::beman::execution::set_error_t(Error)...>>
    : public ::beman::task::detail::result_type<Stop, void_type, Error...> {
  public:
    /*
     * \brief Set the value result although without any value.
     */
    void return_void() { this->set_value(void_type{}); }
};
} // namespace beman::task::detail

// ----------------------------------------------------------------------------

#endif
