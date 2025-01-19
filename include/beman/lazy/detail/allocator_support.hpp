// include/beman/lazy/detail/allocator_support.hpp                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_ALLOCATOR_SUPPORT
#define INCLUDED_BEMAN_LAZY_DETAIL_ALLOCATOR_SUPPORT

#include <beman/lazy/detail/find_allocator.hpp>
#include <memory>
#include <memory_resource>
#include <new>
#include <cstddef>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {

template <typename Allocator>
struct allocator_support {
    template <typename... A>
    void* operator new(std::size_t size, A&&... a) {
        using traits = std::allocator_traits<Allocator>;
        Allocator  alloc{::beman::lazy::detail::find_allocator<Allocator>(a...)};
        std::byte* ptr{traits::allocate(alloc, size + sizeof(Allocator))};
        new (ptr + size) Allocator(alloc);
        return ptr;
    }
    void operator delete(void* ptr, std::size_t size) {
        using traits = std::allocator_traits<Allocator>;
        void*     vptr{static_cast<std::byte*>(ptr) + size};
        auto*     aptr{static_cast<Allocator*>(vptr)};
        Allocator alloc(*aptr);
        aptr->~Allocator();
        traits::deallocate(alloc, static_cast<std::byte*>(ptr), size);
    }
};
} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
