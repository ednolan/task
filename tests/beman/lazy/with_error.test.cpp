// tests/beman/lazy/with_error.test.cpp                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/detail/with_error.hpp>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <concepts>
#include <cassert>

// ----------------------------------------------------------------------------

int main() {
    beman::lazy::detail::with_error we{17};
    static_assert(std::same_as<int, decltype(we)::type>);
    assert(we.error == 17);
}
