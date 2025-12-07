# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.14)

project(smart-test-shared-utils)

add_definitions(-DRUN_ON_LINUX)

set(WAMR_BUILD_LIBC_WASI 0)
set(WAMR_BUILD_APP_FRAMEWORK 0)
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_FAST_INTERP 1)
set(WAMR_BUILD_PLATFORM "linux")
set(WAMR_BUILD_TARGET "X86_64")

include(../../unit_common.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Runtime timer enhanced test
set(RUNTIME_TIMER_ENHANCED_SOURCE
    runtime_timer_enhanced_test.cc
)

# Memory utility functions test (Step 2)
set(MEMORY_UTILITY_STEP2_SOURCE
    shared_utils_add_30_step_2.cc
)

# Assertion and File I/O functions test (Step 3)
set(ASSERTION_FILEIO_STEP3_SOURCE
    shared_utils_add_30_step_3.cc
)

set(unit_test_sources
    ${RUNTIME_TIMER_ENHANCED_SOURCE}
    ${WAMR_RUNTIME_LIB_SOURCE}
)

set(memory_util_test_sources
    ${MEMORY_UTILITY_STEP2_SOURCE}
    ${WAMR_RUNTIME_LIB_SOURCE}
)

set(assertion_fileio_test_sources
    ${ASSERTION_FILEIO_STEP3_SOURCE}
    ${WAMR_RUNTIME_LIB_SOURCE}
    ${WAMR_ROOT_DIR}/core/shared/utils/uncommon/bh_read_file.c
)

add_executable(runtime_timer_enhanced_test ${unit_test_sources})
add_executable(shared_utils_add_30_step_2 ${memory_util_test_sources})
add_executable(shared_utils_add_30_step_3 ${assertion_fileio_test_sources})

target_link_libraries(runtime_timer_enhanced_test gtest_main)
target_link_libraries(shared_utils_add_30_step_2 gtest_main)
target_link_libraries(shared_utils_add_30_step_3 gtest_main)

gtest_discover_tests(runtime_timer_enhanced_test)
gtest_discover_tests(shared_utils_add_30_step_2)
gtest_discover_tests(shared_utils_add_30_step_3)