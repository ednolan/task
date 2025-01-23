// include/beman/lazy/detail/allocator_support.hpp                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_ALLOCATOR_SUPPORT
#define INCLUDED_BEMAN_LAZY_DETAIL_ALLOCATOR_SUPPORT

#include <beman/lazy/detail/find_allocator.hpp>
#include <array>
#include <memory>
#include <new>
#include <cstddef>
#include <cassert>

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
template <typename Allocator>
struct allocator_support {
    using allocator_traits = std::allocator_traits<Allocator>;

    static std::size_t offset(std::size_t size) {
        return (size + alignof(Allocator) - 1u) & ~(alignof(Allocator) - 1u);
    }
    static Allocator* get_allocator(void* ptr, std::size_t size) {
        ptr = static_cast<std::byte*>(ptr) + offset(size);
        return ::std::launder(reinterpret_cast<Allocator*>(ptr));
    }

    template <typename... A>
    void* operator new(std::size_t size, A&&... a) {
        Allocator alloc{::beman::lazy::detail::find_allocator<Allocator>(a...)};
        void*     ptr{allocator_traits::allocate(alloc, offset(size) + sizeof(Allocator))};
        new (get_allocator(ptr, size)) Allocator(alloc);
        return ptr;
    }
    void operator delete(void* ptr, std::size_t size) {
        Allocator* aptr{get_allocator(ptr, size)};
        Allocator  alloc{*aptr};
        aptr->~Allocator();
        // alloc.deallocate(static_cast<std::byte*>(ptr), offset(size) + sizeof(Allocator));
        allocator_traits::deallocate(alloc, static_cast<std::byte*>(ptr), offset(size) + sizeof(Allocator));
    }
};
} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
