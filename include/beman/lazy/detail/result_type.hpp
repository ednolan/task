// include/beman/lazy/detail/result_type.hpp                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_RESULT_TYPE
#define INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_RESULT_TYPE

#include <beman/lazy/detail/sub_visit.hpp>
#include <beman/execution/execution.hpp>
#include <exception>
#include <utility>
#include <type_traits>
#include <variant>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {
/*
 * \brief Helper type used as a placeholder for a void result
 * \headerfile beman/lazy/lazy.hpp <beman/lazy/lazy.hpp>
 * \internal
 */
enum void_type : unsigned char {};
/*
 * \brief Helper type indicating whether a stopped result is possible
 * \headerfile beman/lazy/lazy.hpp <beman/lazy/lazy.hpp>
 * \internal
 */
enum class stoppable { yes, no };

/*
 * \brief Type to hold the result of a coroutine
 * \headerfile beman/lazy/lazy.hpp <beman/lazy/lazy.hpp>
 */
template <::beman::lazy::detail::stoppable Stop, typename Value, typename... Error>
class result_type {
  private:
    ::std::variant<std::monostate, Value, Error...> result;

    template <size_t I, typename E, typename Err, typename... Errs>
    static constexpr ::std::size_t find_index() {
        if constexpr (std::same_as<E, Err>)
            return I;
        else {
            static_assert(0u != sizeof...(Errs), "error type not found in result type");
            return find_index<I + 1u, E, Errs...>();
        }
    }

  public:
    /*
     * \brief Set the result for a `set_value` completion.
     *
     * If `T` is `void_type` the completion is set to become `set_value()`.
     * Otherwise, the `value` becomes the argument of `set_value(value)` when `complete()`
     * is called.
     */
    template <typename T>
    void set_value(T&& value) {
        this->result.template emplace<1u>(::std::forward<T>(value));
    }
    /*
     * \brief Set the result for a `set_error` completion.
     */
    template <typename E>
    void set_error(E&& error) {
        this->result.template emplace<2u + find_index<0u, ::std::remove_cvref_t<E>, Error...>()>(
            ::std::forward<E>(error));
    }

    /*
     * \brief Call the completion function according to the current result.
     *
     * Depending on the current index of the result `complete()` calls the
     * a suitable completion function:
     * - If the index is `0` it calls `set_stopped(std::move(rcvr))`.
     * - If the index is `1` it calls `set_value(std::move(rcvr))` if
     *     the `Value` type is `void_type` and `set_value(std::move(rcvr), std::move(std::get<1>(result)))`
     *     otherwise.
     * - Otherwise it calls `set_error(std::move(rcvr), std::move(std::get<I>(result)))`.
     */
    template <::beman::execution::receiver Receiver>
    void complete(Receiver&& rcvr) {
        switch (this->result.index()) {
        case 0:
            if constexpr (Stop == ::beman::lazy::detail::stoppable::yes)
                ::beman::execution::set_stopped(::std::move(rcvr));
            else
                ::std::terminate();
            break;
        case 1:
            if constexpr (::std::same_as<::beman::lazy::detail::void_type, Value>)
                ::beman::execution::set_value(::std::move(rcvr));
            else
                ::beman::execution::set_value(::std::move(rcvr), ::std::move(::std::get<1u>(this->result)));
            break;
        default:
            if constexpr (0u < sizeof...(Error))
                ::beman::lazy::detail::sub_visit<2u>(
                    [&rcvr](auto& error) { ::beman::execution::set_error(::std::move(rcvr), ::std::move(error)); },
                    this->result);
            break;
        }
    }
};
} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
