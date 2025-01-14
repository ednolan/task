// include/beman/lazy/detail/poly.hpp                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_BEMAN_LAZY_DETAIL_POLY
#define INCLUDED_BEMAN_LAZY_DETAIL_POLY

#include <array>
#include <concepts>
#include <new>
#include <cstddef>

// ----------------------------------------------------------------------------

namespace beman::lazy::detail {
template <typename Base, std::size_t Size>
class alignas(sizeof(double)) poly {
  private:
    std::array<std::byte, Size> buf{};
    Base*                       pointer() { return static_cast<Base*>(static_cast<void*>(buf.data())); }
    const Base* pointer() const { return static_cast<const Base*>(static_cast<const void*>(buf.data())); }

  public:
    template <typename T, typename... Args>
    poly(T*, Args&&... args) {
        new (this->buf.data()) T(::std::forward<Args>(args)...);
        static_assert(sizeof(T) <= Size);
    }
    poly(poly&& other) { other.pointer()->move(this->buf.data()); }
    poly(const poly& other) { other.pointer()->clone(this->buf.data()); }
    ~poly() { this->pointer()->~Base(); }
    bool  operator==(const poly& other) const { return other.pointer()->equals(this->pointer()); }
    Base* operator->() { return this->pointer(); }
};
} // namespace beman::lazy::detail

// ----------------------------------------------------------------------------

#endif
