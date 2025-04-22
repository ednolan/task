# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.25)

project(
    beman.task # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
    DESCRIPTION "A Beman library task (coroutine task)"
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
    execution
    # for local development, use SOURCE_DIR <path-to>/execution
    GIT_REPOSITORY https://github.com/bemanproject/execution
    GIT_TAG 9cd2e66
)
FetchContent_MakeAvailable(execution)

add_subdirectory(src/beman/task)

if(BEMAN_LAZY_BUILD_TESTS)
    add_subdirectory(tests/beman/task)
endif()

if(BEMAN_LAZY_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
