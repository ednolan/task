// include/beman/lazy/detail/allocator.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_ALLOCATOR
#define INCLUDED_BEMAN_LAZY_DETAIL_ALLOCATOR

#include <memory>
#include <memory_resource>
#include <new>
#include <cstddef>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {
template <typename>
struct allocator_of {
    using type = std::allocator<std::byte>;
};
template <typename Context>
    requires requires { typename Context::allocator_type; }
struct allocator_of<Context> {
    using type = typename Context::allocator_type;
};
template <typename Context>
using allocator_of_t = typename allocator_of<Context>::type;

template <typename Allocator>
Allocator find_allocator() {
    return Allocator();
}
template <typename Allocator, typename Alloc, typename... A>
    requires requires(const Alloc& alloc) { Allocator(alloc); }
Allocator find_allocator(const std::allocator_arg_t&, const Alloc& alloc, const A&...) {
    return Allocator(alloc);
}
template <typename Allocator, typename A0, typename... A>
Allocator find_allocator(A0 const&, const A&... a) {
    return ::beman::lazy::detail::find_allocator<Allocator>(a...);
}

template <typename C, typename... A>
void* coroutine_allocate(std::size_t size, const A&... a) {
    using allocator_type = ::beman::lazy::detail::allocator_of_t<C>;
    using traits         = std::allocator_traits<allocator_type>;
    allocator_type alloc{::beman::lazy::detail::find_allocator<allocator_type>(a...)};
    std::byte*     ptr{traits::allocate(alloc, size + sizeof(allocator_type))};
    new (ptr + size) allocator_type(alloc);
    return ptr;
}
template <typename C>
void coroutine_deallocate(void* ptr, std::size_t size) {
    using allocator_type = ::beman::lazy::detail::allocator_of_t<C>;
    using traits         = std::allocator_traits<allocator_type>;
    void*          vptr{static_cast<std::byte*>(ptr) + size};
    auto*          aptr{static_cast<allocator_type*>(vptr)};
    allocator_type alloc(*aptr);
    aptr->~allocator_type();
    traits::deallocate(alloc, static_cast<std::byte*>(ptr), size);
}
} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
