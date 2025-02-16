// tests/beman/task/state_base.test.cpp                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/detail/state_base.hpp>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

// ----------------------------------------------------------------------------

namespace {
struct context {};

struct state : beman::task::detail::state_base<context> {
    stop_source_type source;
    context          ctxt;
    bool             completed{};
    bool             token{};
    bool             got_context{};

    void            do_complete() override { this->completed = true; }
    stop_token_type do_get_stop_token() override {
        this->token = true;
        return this->source.get_token();
    }
    context& do_get_context() override {
        this->got_context = true;
        return this->ctxt;
    }
};
} // namespace

int main() {
    state s;
    assert(s.completed == false);
    s.complete();
    assert(s.completed == true);

    assert(s.token == false);
    s.get_stop_token();
    assert(s.token == true);

    assert(s.got_context == false);
    s.get_context();
    assert(s.got_context == true);
}
