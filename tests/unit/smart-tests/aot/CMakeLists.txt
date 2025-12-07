# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.14)

project(smart-test-aot)

add_definitions(-DRUN_ON_LINUX)
add_definitions(-Dattr_container_malloc=malloc)
add_definitions(-Dattr_container_free=free)
add_definitions(-DWASM_ENABLE_WAMR_COMPILER=1)
add_definitions(-DWASM_ENABLE_DUMP_CALL_STACK=1)
add_definitions(-DWASM_ENABLE_AOT_STACK_FRAME=1)

# AOT-focused build configuration
# Note: INTERP must be enabled because AOT compilation requires interpreter functions
# like wasm_loader_find_block_addr, jit_set_exception_with_id, llvm_jit_invoke_native
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_FAST_INTERP 0)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_LIBC_WASI 0)
set(WAMR_BUILD_APP_FRAMEWORK 1)

include(../../unit_common.cmake)

# LLVM configuration for AOT compilation
set(LLVM_SRC_ROOT "${WAMR_ROOT_DIR}/core/deps/llvm")
if(NOT EXISTS "${LLVM_SRC_ROOT}/build")
  message(FATAL_ERROR "Cannot find LLVM dir: ${LLVM_SRC_ROOT}/build")
endif()
set(CMAKE_PREFIX_PATH "${LLVM_SRC_ROOT}/build;${CMAKE_PREFIX_PATH}")
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include(${IWASM_DIR}/compilation/iwasm_compl.cmake)

# Interpreter directory is already included via WAMR_RUNTIME_LIB_SOURCE
# when WAMR_BUILD_INTERP is 1, but we keep this for explicit clarity
include_directories(${IWASM_DIR}/interpreter)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../common)

file(GLOB_RECURSE source_all ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

set(UNIT_SOURCE ${source_all})

set(unit_test_sources
    ${UNIT_SOURCE}
    ${WAMR_RUNTIME_LIB_SOURCE}
    ${IWASM_COMPL_SOURCE}
)

# Add bh_read_file_to_buffer function
list(APPEND unit_test_sources
    ${WAMR_ROOT_DIR}/core/shared/utils/uncommon/bh_read_file.c
)

# Create enhanced AOT coverage test executable
add_executable(smart-test-aot ${unit_test_sources})

target_link_libraries(smart-test-aot ${LLVM_AVAILABLE_LIBS} gtest_main)

gtest_discover_tests(smart-test-aot)

# Copy WASM files to build directory for testing
add_custom_command(TARGET smart-test-aot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/wasm-apps
    ${CMAKE_CURRENT_BINARY_DIR}/
    COMMENT "Copy test wasm files to the directory of google test"
)