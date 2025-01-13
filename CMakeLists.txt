# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.25)

project(
    beman.lazy # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
    DESCRIPTION "A Beman library lazy (coroutine task)"
    LANGUAGES CXX
)

enable_testing()

# [CMAKE.SKIP_TESTS]
option(
    BEMAN_LAZY_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ON. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    BEMAN_LAZY_BUILD_EXAMPLES
    "Enable building examples. Default: ON. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

include(FetchContent)
include(GNUInstallDirs)

FetchContent_Declare(
    execution26
    # for local development, use SOURCE_DIR <path-to>/execution26
    GIT_REPOSITORY https://github.com/bemanproject/execution26
    GIT_TAG ba3abd3
)
FetchContent_MakeAvailable(execution26)

add_subdirectory(src/beman/lazy)

if(BEMAN_LAZY_BUILD_TESTS)
    add_subdirectory(tests/beman/lazy)
endif()

if(BEMAN_LAZY_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
