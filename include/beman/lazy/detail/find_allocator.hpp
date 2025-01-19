// include/beman/lazy/detail/find_allocator.hpp                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_FIND_ALLOCATOR
#define INCLUDED_INCLUDE_BEMAN_LAZY_DETAIL_FIND_ALLOCATOR

#include <memory>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {
/*!
 * \brief Utility locating an allocator_arg/allocator pair
 * \headerfile beman/lazy/lazy.hpp <beman/lazy/lazy.hpp>
 * \internal
 */
template <typename Allocator>
Allocator find_allocator() {
    return Allocator();
}
template <typename Allocator>
Allocator find_allocator(const std::allocator_arg_t&) {
    static_assert(requires{ { Allocator() } -> std::same_as<void>; }, "There needs to be an allocator argument following std::allocator_arg");
    return Allocator();
}

template <typename Allocator, typename Alloc, typename... A>
Allocator find_allocator(const std::allocator_arg_t&, const Alloc& alloc, const A&...) {
    static_assert(requires(const Alloc& alloc) { Allocator(alloc); },
                  "The allocator needs to be constructible from the argument following std::allocator");
    return Allocator(alloc);
}
template <typename Allocator, typename A0, typename... A>
Allocator find_allocator(A0 const&, const A&... a) {
    return ::beman::lazy::detail::find_allocator<Allocator>(a...);
}

}

// ----------------------------------------------------------------------------

#endif
