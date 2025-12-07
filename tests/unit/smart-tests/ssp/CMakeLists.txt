# Copyright (C) 2025 Intel Corporation. All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.14)

project(smart-test-ssp-unified)

add_definitions(-DRUN_ON_LINUX)

# Enhanced SSP test configuration with platform awareness
set(WAMR_BUILD_LIBC_WASI 1)
set(WAMR_BUILD_APP_FRAMEWORK 0)
set(WAMR_BUILD_AOT 0)
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_FAST_INTERP 0)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_FAST_JIT 0)
set(WAMR_BUILD_MEMORY64 0)
set(WAMR_BUILD_SHARED_MEMORY 0)

# Include common unit test configuration
include(../../unit_common.cmake)

# Include directories for SSP headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${WAMR_ROOT_DIR}/core/iwasm/libraries/libc-wasi/sandboxed-system-primitives/include)
include_directories(${WAMR_ROOT_DIR}/core/iwasm/libraries/libc-wasi/sandboxed-system-primitives/src)

# Create unified test executable
add_executable(smart-test-ssp-unified ssp_unified_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})

# Platform-aware feature detection
if(WAMR_BUILD_TARGET MATCHES "X86_.*")
    target_compile_definitions(smart-test-ssp-unified PRIVATE BUILD_TARGET_X86=1)
    message("-- SSP Unified tests: X86 target detected")
elseif(WAMR_BUILD_TARGET MATCHES "AARCH64.*")
    target_compile_definitions(smart-test-ssp-unified PRIVATE BUILD_TARGET_AARCH64=1)
    message("-- SSP Unified tests: ARM64 target detected")
elseif(WAMR_BUILD_TARGET MATCHES "RISCV.*")
    target_compile_definitions(smart-test-ssp-unified PRIVATE BUILD_TARGET_RISCV=1)
    message("-- SSP Unified tests: RISC-V target detected")
endif()

# Feature-specific definitions (CRITICAL)
if(WAMR_BUILD_SIMD EQUAL 1)
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_SIMD=1)
    message("-- SSP Unified tests: SIMD support enabled")
else()
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_SIMD=0)
endif()

if(WAMR_BUILD_AOT EQUAL 1)
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_AOT=1)
    message("-- SSP Unified tests: AOT support enabled")
else()
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_AOT=0)
endif()

if(WAMR_BUILD_JIT EQUAL 1)
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_JIT=1)
    message("-- SSP Unified tests: JIT support enabled")
else()
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_JIT=0)
endif()

if(WAMR_BUILD_FAST_JIT EQUAL 1)
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_FAST_JIT=1)
    message("-- SSP Unified tests: Fast JIT support enabled")
else()
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_FAST_JIT=0)
endif()

# Memory configuration
if(WAMR_BUILD_MEMORY64 EQUAL 1)
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_MEMORY64=1)
    message("-- SSP Unified tests: Memory64 support enabled")
else()
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_MEMORY64=0)
endif()

if(WAMR_BUILD_SHARED_MEMORY EQUAL 1)
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_SHARED_MEMORY=1)
    message("-- SSP Unified tests: Shared memory support enabled")
else()
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_SHARED_MEMORY=0)
endif()

# Threading support
if(WAMR_BUILD_LIB_PTHREAD EQUAL 1)
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_LIB_PTHREAD=1)
    message("-- SSP Unified tests: Pthread support enabled")
else()
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_LIB_PTHREAD=0)
endif()

# WASI support
if(WAMR_BUILD_LIBC_WASI EQUAL 1)
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_LIBC_WASI=1)
    message("-- SSP Unified tests: LIBC WASI support enabled")
else()
    target_compile_definitions(smart-test-ssp-unified PRIVATE WASM_ENABLE_LIBC_WASI=0)
endif()

# Link libraries
target_link_libraries(smart-test-ssp-unified gtest_main)

# Register with CTest
if(COMMAND gtest_discover_tests)
    gtest_discover_tests(smart-test-ssp-unified)
else()
    add_test(NAME ssp_unified_test COMMAND ssp_unified_test)
endif()

# SSP Step 1 Test - Core File Descriptor Operations
add_executable(ssp_step_1_test ssp_step_1_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})

# Apply same platform and feature configurations to Step 1 test
if(WAMR_BUILD_TARGET MATCHES "X86_.*")
    target_compile_definitions(ssp_step_1_test PRIVATE BUILD_TARGET_X86=1)
elseif(WAMR_BUILD_TARGET MATCHES "AARCH64.*")
    target_compile_definitions(ssp_step_1_test PRIVATE BUILD_TARGET_AARCH64=1)
elseif(WAMR_BUILD_TARGET MATCHES "RISCV.*")
    target_compile_definitions(ssp_step_1_test PRIVATE BUILD_TARGET_RISCV=1)
endif()

if(WAMR_BUILD_LIBC_WASI EQUAL 1)
    target_compile_definitions(ssp_step_1_test PRIVATE WASM_ENABLE_LIBC_WASI=1)
else()
    target_compile_definitions(ssp_step_1_test PRIVATE WASM_ENABLE_LIBC_WASI=0)
endif()

target_link_libraries(ssp_step_1_test gtest_main)

if(COMMAND gtest_discover_tests)
    gtest_discover_tests(ssp_step_1_test)
else()
    add_test(NAME ssp_step_1_test COMMAND ssp_step_1_test)
endif()

# SSP Step 2 Test - File Descriptor Management & Metadata Operations  
add_executable(ssp_step_2_test ssp_step_2_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})

# Apply same platform and feature configurations to Step 2 test
if(WAMR_BUILD_TARGET MATCHES "X86_.*")
    target_compile_definitions(ssp_step_2_test PRIVATE BUILD_TARGET_X86=1)
elseif(WAMR_BUILD_TARGET MATCHES "AARCH64.*")
    target_compile_definitions(ssp_step_2_test PRIVATE BUILD_TARGET_AARCH64=1)
elseif(WAMR_BUILD_TARGET MATCHES "RISCV.*")
    target_compile_definitions(ssp_step_2_test PRIVATE BUILD_TARGET_RISCV=1)
endif()

if(WAMR_BUILD_LIBC_WASI EQUAL 1)
    target_compile_definitions(ssp_step_2_test PRIVATE WASM_ENABLE_LIBC_WASI=1)
else()
    target_compile_definitions(ssp_step_2_test PRIVATE WASM_ENABLE_LIBC_WASI=0)
endif()

target_link_libraries(ssp_step_2_test gtest_main)

if(COMMAND gtest_discover_tests)
    gtest_discover_tests(ssp_step_2_test)
else()
    add_test(NAME ssp_step_2_test COMMAND ssp_step_2_test)
endif()

# SSP Step 3 Test - Socket Operations & Network Functions
add_executable(ssp_step_3_test ssp_step_3_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})

# Apply same platform and feature configurations to Step 3 test
if(WAMR_BUILD_TARGET MATCHES "X86_.*")
    target_compile_definitions(ssp_step_3_test PRIVATE BUILD_TARGET_X86=1)
elseif(WAMR_BUILD_TARGET MATCHES "AARCH64.*")
    target_compile_definitions(ssp_step_3_test PRIVATE BUILD_TARGET_AARCH64=1)
elseif(WAMR_BUILD_TARGET MATCHES "RISCV.*")
    target_compile_definitions(ssp_step_3_test PRIVATE BUILD_TARGET_RISCV=1)
endif()

if(WAMR_BUILD_LIBC_WASI EQUAL 1)
    target_compile_definitions(ssp_step_3_test PRIVATE WASM_ENABLE_LIBC_WASI=1)
else()
    target_compile_definitions(ssp_step_3_test PRIVATE WASM_ENABLE_LIBC_WASI=0)
endif()

target_link_libraries(ssp_step_3_test gtest_main)

if(COMMAND gtest_discover_tests)
    gtest_discover_tests(ssp_step_3_test)
else()
    add_test(NAME ssp_step_3_test COMMAND ssp_step_3_test)
endif()

# Enhanced Blocking Op Test - Core Blocking Operations
add_executable(enhanced_blocking_op_test enhanced_blocking_op_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})

# Apply same platform and feature configurations to Enhanced Blocking Op test
if(WAMR_BUILD_TARGET MATCHES "X86_.*")
    target_compile_definitions(enhanced_blocking_op_test PRIVATE BUILD_TARGET_X86=1)
elseif(WAMR_BUILD_TARGET MATCHES "AARCH64.*")
    target_compile_definitions(enhanced_blocking_op_test PRIVATE BUILD_TARGET_AARCH64=1)
elseif(WAMR_BUILD_TARGET MATCHES "RISCV.*")
    target_compile_definitions(enhanced_blocking_op_test PRIVATE BUILD_TARGET_RISCV=1)
endif()

if(WAMR_BUILD_LIBC_WASI EQUAL 1)
    target_compile_definitions(enhanced_blocking_op_test PRIVATE WASM_ENABLE_LIBC_WASI=1)
else()
    target_compile_definitions(enhanced_blocking_op_test PRIVATE WASM_ENABLE_LIBC_WASI=0)
endif()

target_link_libraries(enhanced_blocking_op_test gtest_main)

if(COMMAND gtest_discover_tests)
    gtest_discover_tests(enhanced_blocking_op_test)
else()
    add_test(NAME enhanced_blocking_op_test COMMAND enhanced_blocking_op_test)
endif()

# Enhanced POSIX Test - Core POSIX Functions
add_executable(enhanced_posix_test enhanced_posix_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})

# Apply same platform and feature configurations to Enhanced POSIX test
if(WAMR_BUILD_TARGET MATCHES "X86_.*")
    target_compile_definitions(enhanced_posix_test PRIVATE BUILD_TARGET_X86=1)
elseif(WAMR_BUILD_TARGET MATCHES "AARCH64.*")
    target_compile_definitions(enhanced_posix_test PRIVATE BUILD_TARGET_AARCH64=1)
elseif(WAMR_BUILD_TARGET MATCHES "RISCV.*")
    target_compile_definitions(enhanced_posix_test PRIVATE BUILD_TARGET_RISCV=1)
endif()

if(WAMR_BUILD_LIBC_WASI EQUAL 1)
    target_compile_definitions(enhanced_posix_test PRIVATE WASM_ENABLE_LIBC_WASI=1)
else()
    target_compile_definitions(enhanced_posix_test PRIVATE WASM_ENABLE_LIBC_WASI=0)
endif()

target_link_libraries(enhanced_posix_test gtest_main)

if(COMMAND gtest_discover_tests)
    gtest_discover_tests(enhanced_posix_test)
else()
    add_test(NAME enhanced_posix_test COMMAND enhanced_posix_test)
endif()

message("-- SSP Enhanced tests: Created Step 1, Step 2, and Step 3 test executables")
message("-- SSP Enhanced tests: Created Enhanced Blocking Op test executable")
message("-- SSP Enhanced tests: Created Enhanced POSIX test executable")
message("-- SSP Unified tests: Created single unified test executable")