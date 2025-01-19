// tests/beman/lazy/allocator_of.test.cpp                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/detail/allocator_of.hpp>
#include <memory_resource>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

// ----------------------------------------------------------------------------

namespace {
    struct no_allocator {};
    struct defines_allocator {
        using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
    };
}

int main() {

}
