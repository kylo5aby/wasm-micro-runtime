# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.14)

project(smart-test-simd-opcodes)

set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_JIT 1)
set(WAMR_BUILD_FAST_JIT 0)
set(WAMR_BUILD_LIBC_WASI 1)
set(WAMR_BUILD_APP_FRAMEWORK 0)
set(WAMR_BUILD_MEMORY_PROFILING 0)

set(WAMR_BUILD_SIMD 1)

# Platform-specific definitions
add_definitions(-DRUN_ON_LINUX)

# Include shared unit test configuration
include(../../unit_common.cmake)

# Configure LLVM for AOT compilation support
set(LLVM_SRC_ROOT "${WAMR_ROOT_DIR}/core/deps/llvm")

if (NOT EXISTS "${LLVM_SRC_ROOT}/build")
    message(FATAL_ERROR "Cannot find LLVM dir: ${LLVM_SRC_ROOT}/build")
endif ()

set(CMAKE_PREFIX_PATH "${LLVM_SRC_ROOT}/build;${CMAKE_PREFIX_PATH}")
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include AOT compilation support
include(${IWASM_DIR}/compilation/iwasm_compl.cmake)

# Set up include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Collect all test source files
file(GLOB_RECURSE source_all ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
# Remove files from build directory to avoid conflicts
list(FILTER source_all EXCLUDE REGEX "${CMAKE_CURRENT_BINARY_DIR}/.*")

# Ensure f32x4.extract_lane test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_f32x4_extract_lane_test.cc)

# Ensure f64x2.extract_lane test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_f64x2_extract_lane_test.cc)

# Ensure i64x2.shl test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i64x2_shl_test.cc)

# Ensure i32x4.extadd_pairwise_i16x8_u test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i32x4_extadd_pairwise_i16x8_u_test.cc)

# Ensure i16x8.q15mulr_sat_s test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i16x8_q15mulr_sat_s_test.cc)

# Ensure f64x2.convert_low_i32x4_s test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_f64x2_convert_low_i32x4_s_test.cc)

# Ensure f32x4.neg test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_f32x4_neg_test.cc)
# Ensure i8x16.avgr_u test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i8x16_avgr_u_test.cc)

# Ensure i32x4.lt_s test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i32x4_lt_s_test.cc)

# Ensure i16x8.narrow_i32x4_u test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i16x8_narrow_i32x4_u_test.cc)

# Ensure i16x8.extend_low_i8x16_s test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i16x8_extend_low_i8x16_s_test.cc)

# Ensure f64x2.promote_low_f32x4 test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_f64x2_promote_low_f32x4_test.cc)

# Ensure i64x2.gt_u test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i64x2_gt_u_test.cc)

# Ensure i8x16.narrow_i16x8_s test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i8x16_narrow_i16x8_s_test.cc)

# Ensure i64x2.extend_high_i32x4 test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i64x2_extend_high_i32x4_test.cc)

# Ensure i32x4.extend_low_i16x8 test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i32x4_extend_low_i16x8_test.cc)

# Ensure i16x8.extadd_pairwise_i8x16 test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i16x8_extadd_pairwise_i8x16_test.cc)

# Ensure i64x2.extmul_low_i32x4 test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i64x2_extmul_low_i32x4_test.cc)

# Ensure f32x4.abs test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_f32x4_abs_test.cc)

# Ensure f64x2.neg test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_f64x2_neg_test.cc)

# Ensure f32x4.div test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_f32x4_div_test.cc)

# Ensure f64x2.sub test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_f64x2_sub_test.cc)

# Ensure i64x2.sub test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i64x2_sub_test.cc)

# Ensure i8x16.sub test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i8x16_sub_test.cc)

# Ensure i32x4.lt test is included
list(APPEND source_all ${CMAKE_CURRENT_SOURCE_DIR}/enhanced_i32x4_lt_test.cc)

set(UNIT_SOURCE ${source_all})

# Prepare unit test sources
set(unit_test_sources
        ${UNIT_SOURCE}
        ${WAMR_RUNTIME_LIB_SOURCE}
        ${UNCOMMON_SHARED_SOURCE}
        )

# Create test executable
add_executable(smart-test-simd-opcodes ${unit_test_sources})

# Link required libraries
target_link_libraries(smart-test-simd-opcodes ${LLVM_AVAILABLE_LIBS} gtest_main)
# Enable test discovery
include(GoogleTest)
gtest_discover_tests(smart-test-simd-opcodes)

# Copy WASM test files to build directory for test execution
add_custom_command(TARGET smart-test-simd-opcodes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/wasm-apps
    ${CMAKE_CURRENT_BINARY_DIR}/wasm-apps
    COMMENT "Copying WASM test files to build directory"
)
