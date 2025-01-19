// include/beman/lazy/detail/allocator_support.hpp                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_ALLOCATOR_SUPPORT
#define INCLUDED_BEMAN_LAZY_DETAIL_ALLOCATOR_SUPPORT

#include <beman/lazy/detail/find_allocator.hpp>
#include <array>
#include <memory>
#include <new>
#include <cstddef>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {
/*!
 * \brief Utility adding allocator support to type by embedding the allocator
 * \headerfile beman/lazy/lazy.hpp <beman/lazy/lazy.hpp>
 *
 * To add allocator support using this class just publicly inherit from
 * allocator_support<Allocator, YourPromiseType>. This utility is probably
 * only useful for coroutine promise types.
 *
 * This struct is a massive hack, primarily support allocators for coroutines.
 * The memory for coroutines is implicitly managed and there isn't a way to
 * provide the memory directly. Instead, the promise_type can overload an
 * operator new and somehow determine an allocator based on the arguments
 * passed to the coroutine. Even worse, the operator delete only gets passed
 * a pointer to delete and a size. To determine the correct allocator the
 * operator delete needs to located it based on this information. Putting
 * the allocator after actually used memory causes the address sanitizer to
 * object! So, the current strategy is to embed space for the allocator
 * into the object and pull it out from there.
 */
template <typename Allocator, typename Derived>
struct allocator_support {
    std::array<std::byte, sizeof(Allocator)> buffer;
    allocator_support() {}

    template <typename... A>
    void* operator new(std::size_t size, A&&... a) {
        using traits = std::allocator_traits<Allocator>;
        Allocator                              alloc{::beman::lazy::detail::find_allocator<Allocator>(a...)};
        void*                                  ptr{traits::allocate(alloc, size)};
        allocator_support<Allocator, Derived>* support{static_cast<Derived*>(ptr)};
        new (support->buffer.data()) Allocator(alloc);
        return ptr;
    }
    void operator delete(void* ptr, std::size_t size) {
        using traits = std::allocator_traits<Allocator>;
        allocator_support<Allocator, Derived>* support{static_cast<Derived*>(ptr)};
        void*                                  vptr{support->buffer.data()};
        auto*                                  aptr{static_cast<Allocator*>(vptr)};
        Allocator                              alloc(*aptr);
        aptr->~Allocator();
        traits::deallocate(alloc, static_cast<std::byte*>(ptr), size);
    }
};
} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
