// examples/any_scheduler.cpp                                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/task/task.hpp>
#include <beman/execution/execution.hpp>
#include <iostream>
#include <cstdlib>

namespace ex = beman::execution;
namespace ts = beman::task;

int main() {
    ts::inline_scheduler       isched;
    const ts::inline_scheduler cisched;
    ts::any_scheduler          asched(isched);
    const ts::any_scheduler    casched(cisched);

    return isched == isched && asched == asched && asched == isched && isched == asched && asched == cisched &&
                   cisched == casched && !(cisched != casched) && !(asched != isched)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
