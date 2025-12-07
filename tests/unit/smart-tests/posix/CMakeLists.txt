# Copyright (C) 2024 Intel Corporation. All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.14)

project(smart-test-posix)

add_definitions(-DRUN_ON_LINUX)

set(WAMR_BUILD_LIBC_WASI 1)
set(WAMR_BUILD_APP_FRAMEWORK 0)
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_FAST_INTERP 1)
set(WAMR_BUILD_SHARED_MEMORY 1)
set(WAMR_BUILD_PLATFORM "linux")
set(WAMR_BUILD_TARGET "X86_64")

# Include common unit test configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/../../unit_common.cmake)

# Set test executable name for +23% coverage improvement
set(SMART_TEST_POSIX_TEST_NAME smart-test-posix)

# Include directories for POSIX platform headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Enhanced test source files (step-by-step implementation for +23% coverage)
set(SMART_TEST_POSIX_SOURCES
    posix_add_23_step_1.cc  # Core File I/O Operations (15 functions, ~280 lines)
    posix_add_23_step_2.cc  # Network Socket Operations (12 functions, ~220 lines)
    posix_add_23_step_3.cc  # Memory and Thread Operations (10 functions, ~180 lines)
    posix_add_23_step_4.cc  # System and Time Operations (8 functions, ~120 lines)
)

# Filter source files that actually exist
set(EXISTING_SOURCES)
foreach(SOURCE ${SMART_TEST_POSIX_SOURCES})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE})
        list(APPEND EXISTING_SOURCES ${SOURCE})
    endif()
endforeach()

# Only create test if we have source files
if(EXISTING_SOURCES)
    # Include WAMR runtime library sources
    set(unit_test_sources
        ${EXISTING_SOURCES}
        ${WAMR_RUNTIME_LIB_SOURCE}
        ${UNCOMMON_SHARED_SOURCE}
    )
    
    # Create enhanced test executable
    add_executable(${SMART_TEST_POSIX_TEST_NAME} ${unit_test_sources})
    
    # Link with required libraries
    target_link_libraries(${SMART_TEST_POSIX_TEST_NAME} gtest_main)
    
    # Platform-specific linking for POSIX features
    if(UNIX AND NOT APPLE)
        target_link_libraries(${SMART_TEST_POSIX_TEST_NAME} pthread dl m)
    elseif(APPLE)
        target_link_libraries(${SMART_TEST_POSIX_TEST_NAME} pthread dl m)
    endif()
    
    # Register with GTest discovery
    gtest_discover_tests(${SMART_TEST_POSIX_TEST_NAME})
    
    # Set test properties for coverage collection
    set_target_properties(${SMART_TEST_POSIX_TEST_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
    )
    
    # Add coverage compilation flags if enabled
    if(COLLECT_CODE_COVERAGE)
        target_compile_options(${SMART_TEST_POSIX_TEST_NAME} PRIVATE --coverage)
        target_link_options(${SMART_TEST_POSIX_TEST_NAME} PRIVATE --coverage)
    endif()
    
    list(LENGTH EXISTING_SOURCES source_count)
    message(STATUS "Smart test POSIX coverage tests configured with ${source_count} source files")
else()
    message(STATUS "No enhanced POSIX test source files found - skipping test creation")
endif()

# Create test data directory if needed
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_data)

# Include IWASM compilation configuration
include(${IWASM_DIR}/compilation/iwasm_compl.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../common)

# Collect all test source files
file(GLOB_RECURSE source_all ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

set(UNIT_SOURCE ${source_all})

aux_source_directory(. SRC_LIST)

set(unit_test_sources
    ${UNIT_SOURCE}
    ${WAMR_RUNTIME_LIB_SOURCE}
    ${UNCOMMON_SHARED_SOURCE}
    ${SRC_LIST}
)

# Create POSIX enhanced test executable
add_executable(smart-test-posix ${unit_test_sources})

target_link_libraries(smart-test-posix ${LLVM_AVAILABLE_LIBS} gtest_main -lm -ldl -lpthread)

gtest_discover_tests(smart-test-posix)
