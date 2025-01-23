// tests/beman/lazy/allocator_support.test.cpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/lazy/detail/allocator_support.hpp>
#include <memory_resource>
#include <new>
#include <cstddef>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

// ----------------------------------------------------------------------------

namespace {
struct test_resource : std::pmr::memory_resource {
    std::size_t outstanding{};

    virtual void* do_allocate(std::size_t size, std::size_t) override {
        this->outstanding += size;
        return ::operator new(size);
    }
    virtual void do_deallocate(void* ptr, std::size_t size, std::size_t align) override {
        if constexpr (requires{ ::operator delete(ptr, size); })
            ::operator delete(ptr, size);
        else
            ::operator delete(ptr);

        this->outstanding -= size;
    }
    bool do_is_equal(const memory_resource& other) const noexcept override {
        auto* tr{dynamic_cast<const test_resource*>(&other)};
        return tr == this;
    }
};

struct some_data {
    double data{};
};

template <typename Allocator>
struct allocator_aware : some_data, beman::lazy::detail::allocator_support<Allocator> {
    allocator_aware() : some_data() {}
};
} // namespace

int main() {
    delete new allocator_aware<std::pmr::polymorphic_allocator<std::byte>>{};
    delete new (std::allocator_arg, std::pmr::polymorphic_allocator<std::byte>{})
        allocator_aware<std::pmr::polymorphic_allocator<std::byte>>{};

    test_resource resource{};
    assert(resource.outstanding == 0u);
    auto ptr{new (std::allocator_arg, &resource)
                 allocator_aware<std::pmr::polymorphic_allocator<std::byte>>()};
    assert(resource.outstanding != 0u);
    delete ptr;
    assert(resource.outstanding == 0u);
}
