# Copyright (C) 2019 Intel Corporation. All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.14)

project(smart-test-aot-arch)

# Add the current directory to the module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Set WAMR build configuration BEFORE including unit_common.cmake
# These must be set before config_common.cmake is included
# Note: INTERP must be enabled because AOT compilation requires interpreter functions
set(WAMR_BUILD_PLATFORM "linux")
set(WAMR_BUILD_TARGET "X86_64")
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_FAST_INTERP 0)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_LIBC_WASI 0)
set(WAMR_BUILD_LIBC_BUILTIN 1)
set(WAMR_BUILD_APP_FRAMEWORK 0)

# Include the build config
include(${CMAKE_CURRENT_LIST_DIR}/../../unit_common.cmake)

# Set up LLVM for AOT compilation support
set (LLVM_SRC_ROOT "${WAMR_ROOT_DIR}/core/deps/llvm")
if (NOT EXISTS "${LLVM_SRC_ROOT}/build")
  message (FATAL_ERROR "Cannot find LLVM dir: ${LLVM_SRC_ROOT}/build")
endif ()
set (CMAKE_PREFIX_PATH "${LLVM_SRC_ROOT}/build;${CMAKE_PREFIX_PATH}")
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Set test target name
set(TEST_TARGET smart-test-aot-arch)

# Create test executable for Step 1
set(STEP1_TARGET smart-test-aot-arch-step1)

# Define source files needed for AOT relocation testing
set (unit_test_sources
     aot_arch_add_40_step_1.cc
     ${PLATFORM_SHARED_SOURCE}
     ${UTILS_SHARED_SOURCE}
     ${MEM_ALLOC_SHARED_SOURCE}
     ${NATIVE_INTERFACE_SOURCE}
     ${LIBC_BUILTIN_SOURCE}
     ${LIBC_WASI_SOURCE}
     ${IWASM_COMMON_SOURCE}
     ${IWASM_INTERP_SOURCE}
     ${IWASM_AOT_SOURCE}
)

add_executable(${STEP1_TARGET} ${unit_test_sources})

# Platform-aware feature detection and compilation definitions
message("=== AOT Architecture Test Platform Configuration ===")

# Detect current WAMR configuration from parent build
if(WAMR_BUILD_TARGET MATCHES "X86_.*")
    target_compile_definitions(${STEP1_TARGET} PRIVATE BUILD_TARGET_X86_64=1)
    message("-- AOT Arch tests: X86_64 target detected")
elseif(WAMR_BUILD_TARGET MATCHES "AARCH64.*")
    target_compile_definitions(${STEP1_TARGET} PRIVATE BUILD_TARGET_AARCH64=1)
    message("-- AOT Arch tests: ARM64 target detected")
elseif(WAMR_BUILD_TARGET MATCHES "RISCV.*")
    target_compile_definitions(${STEP1_TARGET} PRIVATE BUILD_TARGET_RISCV=1)
    message("-- AOT Arch tests: RISC-V target detected")
else()
    # Default to X86_64 for unit tests
    target_compile_definitions(${STEP1_TARGET} PRIVATE BUILD_TARGET_X86_64=1)
    message("-- AOT Arch tests: Defaulting to X86_64 target")
endif()

# Feature-specific definitions (CRITICAL for platform compatibility)
if(WAMR_BUILD_SIMD EQUAL 1)
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_SIMD=1)
    message("-- AOT Arch tests: SIMD support enabled")
else()
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_SIMD=0)
endif()

if(WAMR_BUILD_AOT EQUAL 1)
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_AOT=1)
    message("-- AOT Arch tests: AOT support enabled")
else()
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_AOT=0)
endif()

if(WAMR_BUILD_JIT EQUAL 1)
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_JIT=1)
    message("-- AOT Arch tests: JIT support enabled")
else()
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_JIT=0)
endif()

if(WAMR_BUILD_FAST_JIT EQUAL 1)
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_FAST_JIT=1)
    message("-- AOT Arch tests: Fast JIT support enabled")
else()
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_FAST_JIT=0)
endif()

# Memory configuration
if(WAMR_BUILD_MEMORY64 EQUAL 1)
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_MEMORY64=1)
    message("-- AOT Arch tests: Memory64 support enabled")
else()
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_MEMORY64=0)
endif()

if(WAMR_BUILD_SHARED_MEMORY EQUAL 1)
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_SHARED_MEMORY=1)
    message("-- AOT Arch tests: Shared memory support enabled")
else()
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_SHARED_MEMORY=0)
endif()

# Threading support
if(WAMR_BUILD_LIB_PTHREAD EQUAL 1)
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_LIB_PTHREAD=1)
    message("-- AOT Arch tests: Pthread support enabled")
else()
    target_compile_definitions(${STEP1_TARGET} PRIVATE WASM_ENABLE_LIB_PTHREAD=0)
endif()

message("================================================")

# Include directories for WAMR headers
target_include_directories(${STEP1_TARGET} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../../../core/iwasm/include
    ${CMAKE_CURRENT_LIST_DIR}/../../../../core/iwasm/common
    ${CMAKE_CURRENT_LIST_DIR}/../../../../core/iwasm/aot
    ${CMAKE_CURRENT_LIST_DIR}/../../../../core/iwasm/aot/arch
    ${CMAKE_CURRENT_LIST_DIR}/../../../../core/shared/platform/linux
    ${CMAKE_CURRENT_LIST_DIR}/../../../../core/shared/platform/common/posix
    ${CMAKE_CURRENT_LIST_DIR}/../../../../core/shared/mem-alloc
    ${CMAKE_CURRENT_LIST_DIR}/../../../../core/shared/utils
    ${CMAKE_CURRENT_LIST_DIR}/../..
)

# Link against WAMR libraries and LLVM
target_link_libraries(${STEP1_TARGET}
    ${LLVM_AVAILABLE_LIBS}
    gtest_main
)

# Add platform-specific link options
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${STEP1_TARGET} pthread dl m)
endif()

# Enable code coverage if requested
if(COLLECT_CODE_COVERAGE EQUAL 1)
    target_compile_options(${STEP1_TARGET} PRIVATE --coverage)
    target_link_options(${STEP1_TARGET} PRIVATE --coverage)
endif()

# Add test to CTest
gtest_discover_tests(${STEP1_TARGET})

# Copy any required WASM files to build directory (for future steps)
add_custom_command(TARGET ${STEP1_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Smart Test AOT Architecture Step 1 tests built successfully"
    COMMENT "Smart Test AOT Architecture Step 1 test build complete"
)

# Add the test to the main test suite
add_test(NAME smart-test-aot-arch COMMAND ${STEP1_TARGET})

# Set test properties for better output
set_tests_properties(smart-test-aot-arch PROPERTIES
    TIMEOUT 300
    LABELS "smart-test-aot-arch;coverage"
)