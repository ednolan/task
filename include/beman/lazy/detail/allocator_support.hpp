// include/beman/lazy/detail/allocator_support.hpp                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_ALLOCATOR_SUPPORT
#define INCLUDED_BEMAN_LAZY_DETAIL_ALLOCATOR_SUPPORT

#include <beman/lazy/detail/find_allocator.hpp>
#include <array>
#include <memory>
#include <memory_resource>
#include <new>
#include <cstddef>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {

template <typename Allocator, typename Derived>
struct allocator_support {
    std::array<std::byte, sizeof(Allocator)> buffer;

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
