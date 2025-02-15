# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

add_library(beman.lazy STATIC)
add_library(beman::lazy ALIAS beman.lazy)

target_sources(beman.lazy PRIVATE task.cpp)

target_sources(
    beman.lazy
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${PROJECT_SOURCE_DIR}/include
        FILES ${PROJECT_SOURCE_DIR}/include/beman/lazy/task.hpp
)

set_target_properties(beman.lazy PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON)
target_link_libraries(beman.lazy PUBLIC beman::execution)

install(
    TARGETS beman.lazy
    EXPORT beman.lazy
    DESTINATION
    $<$<CONFIG:Debug>:debug/>${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION $<$<CONFIG:Debug>:debug/>${CMAKE_INSTALL_BINDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
