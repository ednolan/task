// examples/alloc.cpp                                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <beman/execution26/execution.hpp>
#include <beman/lazy/lazy.hpp>

namespace ex = beman::execution26;

#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define TSAN_ENABLED
#endif
#endif
#if !defined(TSAN_ENABLED)
void* operator new(std::size_t size) {
    void* pointer(std::malloc(size));
    std::cout << "global new(" << size << ")->" << pointer << "\n";
    return pointer;
}
void operator delete(void* pointer) noexcept {
    std::cout << "global delete(" << pointer << ")\n";
    return std::free(pointer);
}
void operator delete(void* pointer, std::size_t size) noexcept {
    std::cout << "global delete(" << pointer << ", " << size << ")\n";
    return std::free(pointer);
}
#endif

template <std::size_t Size>
class fixed_resource : public std::pmr::memory_resource {
    std::byte  buffer[Size];
    std::byte* free{this->buffer};

    void* do_allocate(std::size_t size, std::size_t) override {
        if (size <= std::size_t(buffer + Size - free)) {
            auto ptr{this->free};
            this->free += size;
            std::cout << "resource alloc(" << size << ")->" << ptr << "\n";
            return ptr;
        } else {
            return nullptr;
        }
    }
    void do_deallocate(void* ptr, std::size_t size, std::size_t) override {
        std::cout << "resource dealloc(" << size << "+" << sizeof(std::pmr::polymorphic_allocator<std::byte>) << ")->"
                  << ptr << "\n";
    }
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override { return &other == this; }
};

struct alloc_aware {
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
};

int main() {
    std::cout << "running just\n";
    ex::sync_wait(ex::just(0));
    std::cout << "just done\n\n";

    std::cout << "running ex::lazy<void, alloc_aware>\n";
    ex::sync_wait([]() -> ex::lazy<void, alloc_aware> {
        co_await ex::just(0);
        co_await ex::just(0);
    }());
    std::cout << "ex::lazy<void, alloc_aware> done\n\n";

    fixed_resource<2048> resource;
    std::cout << "running ex::lazy<void, alloc_aware> with alloc\n";
    ex::sync_wait([](auto&&...) -> ex::lazy<void, alloc_aware> {
        co_await ex::just(0);
        co_await ex::just(0);
    }(std::allocator_arg, &resource));
    std::cout << "ex::lazy<void, alloc_aware> with alloc done\n\n";

    std::cout << "running ex::lazy<void> with alloc\n";
    ex::sync_wait([](auto&&...) -> ex::lazy<void> {
        co_await ex::just(0);
        co_await ex::just(0);
    }(std::allocator_arg, &resource));
    std::cout << "ex::lazy<void> with alloc done\n\n";

    std::cout << "running ex::lazy<void, alloc_aware> extracting alloc\n";
    ex::sync_wait([](auto&&, auto* resource) -> ex::lazy<void, alloc_aware> {
        auto alloc = co_await ex::read_env(ex::get_allocator);
        static_assert(std::same_as<std::pmr::polymorphic_allocator<std::byte>, decltype(alloc)>);
        assert(alloc == std::pmr::polymorphic_allocator<std::byte>(resource));
    }(std::allocator_arg, &resource));
    std::cout << "ex::lazy<void, alloc_aware> extracting alloc done\n\n";
}
