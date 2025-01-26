<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# beman.lazy: Beman Library Implementation of `lazy` (P3552)

![Continuous Integration Tests](https://github.com/bemanproject/lazy/actions/workflows/ci_tests.yml/badge.svg)

`beman::execution::lazy<T, Context>` is a class template which
is used as the the type of coroutine tasks. The corresponding objects
are senders.  The first template argument (`T`) defines the result
type which becomes a `set_value_t(T)` completion signatures. The
second template argument (`Context`) provides a way to configure
the behavior of the coroutine. By default it can be left alone.

Implements: `std::execution::lazy` proposed in [Add a Coroutine Lazy Type (P3552r0)](https://wg21.link/P3552r0).

## Usage

The following code snippet shows a basic use of `beman::lazy::lazy`
using sender/receiver facilities to implement version of `hello,
world`:

```cpp
#include <beman/lazy/lazy.hpp>
#include <beman/execution/execution.hpp>
#include <iostream>

namespace ex = beman::execution;
namespace ly = beman::lazy;

int main() {
    return std::get<0>(*ex::sync_wait([]->ex::lazy<int> {
        std::cout << "Hello, world!\n";
        co_return co_await ex::just(0);
    }()));
}
```

Full runnable examples can be found in `examples/` (e.g., [`./examples/hello.cpp`](./examples/hello.cpp)).

## Help Welcome

There are plenty of things which need to be done. See the
[contributions page](https://github.com/bemanproject/lazy/blob/main/docs/contributing.md)
for some ideas how to contribute. The [resources page](https://github.com/bemanproject/lazy/blob/main/docs/resources.md)
contains some links for general information about coroutines.

## Building beman.lazy

### Dependencies

This project depends on
[`beman::execution`](https://bemanproject/execution) (which
will be automatically obtained using `cmake`'s `FetchContent*`).

Build-time dependencies:

- `cmake`
- `ninja`, `make`, or another CMake-supported build system
  - CMake defaults to "Unix Makefiles" on POSIX systems

### How to build beman.lazy

This project strives to be as normal and simple a CMake project as
possible.  This build workflow in particular will work, producing
a static `libbeman.lazy.a` library, ready to package with its
headers:

```shell
cmake --workflow --preset gcc-debug
cmake --workflow --preset gcc-release
cmake --install build/gcc-release --prefix /opt/beman.lazy
```

## Contributing

Please do! Issues and pull requests are appreciated.
