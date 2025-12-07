# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.14)

project (smart-test-compilation)

add_definitions (-DRUN_ON_LINUX)

add_definitions (-Dattr_container_malloc=malloc)
add_definitions (-Dattr_container_free=free)
add_definitions (-DWASM_ENABLE_WAMR_COMPILER=1)
add_definitions (-DWASM_ENABLE_DUMP_CALL_STACK=1)
add_definitions (-DWASM_ENABLE_AOT_STACK_FRAME=1)

# Set WAMR build configuration BEFORE including unit_common.cmake
# These must be set before config_common.cmake is included
# Note: INTERP must be enabled because compilation requires interpreter functions
set (WAMR_BUILD_PLATFORM "linux")
set (WAMR_BUILD_TARGET "X86_64")
set (WAMR_BUILD_LIBC_WASI 0)
set (WAMR_BUILD_APP_FRAMEWORK 0)
set (WAMR_BUILD_THREAD_MGR 1)
set (WAMR_BUILD_AOT 1)
set (WAMR_BUILD_INTERP 1)
# Note: SIMD is disabled because compilation module has compatibility issues with FAST_INTERP
# The compilation module accesses WASMInterpFrame.sp which doesn't exist in FAST_INTERP
# SIMD requires FAST_INTERP, so we disable SIMD to use CLASSIC_INTERP
# TODO: This is a known WAMR limitation - compilation module needs to be updated for FAST_INTERP support
set (WAMR_BUILD_FAST_INTERP 0)
set (WAMR_BUILD_JIT 0)
set (WAMR_BUILD_SIMD 0)

include (../../unit_common.cmake)

set (LLVM_SRC_ROOT "${WAMR_ROOT_DIR}/core/deps/llvm")
if (NOT EXISTS "${LLVM_SRC_ROOT}/build")
  message (FATAL_ERROR "Cannot find LLVM dir: ${LLVM_SRC_ROOT}/build")
endif ()
set (CMAKE_PREFIX_PATH "${LLVM_SRC_ROOT}/build;${CMAKE_PREFIX_PATH}")
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include (${IWASM_DIR}/compilation/iwasm_compl.cmake)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

file (GLOB source_all ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

set (UNIT_SOURCE ${source_all})

set (unit_test_sources
     ${UNIT_SOURCE}
     ${WAMR_RUNTIME_LIB_SOURCE}
     ${UNCOMMON_SHARED_SOURCE}
     ${SRC_LIST}
     ${PLATFORM_SHARED_SOURCE}
     ${UTILS_SHARED_SOURCE}
     ${MEM_ALLOC_SHARED_SOURCE}
     ${LIB_HOST_AGENT_SOURCE}
     ${NATIVE_INTERFACE_SOURCE}
     ${LIBC_BUILTIN_SOURCE}
     ${IWASM_COMMON_SOURCE}
     ${IWASM_INTERP_SOURCE}
     ${IWASM_AOT_SOURCE}
     ${IWASM_COMPL_SOURCE}
     ${WASM_APP_LIB_SOURCE_ALL}
    )

# Now simply link against gtest or gtest_main as needed. Eg
add_executable (smart-test-compilation ${unit_test_sources})

target_link_libraries (smart-test-compilation ${LLVM_AVAILABLE_LIBS} gtest_main )

add_custom_command(TARGET smart-test-compilation POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_LIST_DIR}/wasm-apps/main.wasm
  ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_LIST_DIR}/wasm-apps/simd_test.wasm
  ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_LIST_DIR}/wasm-apps/llvm_test.wasm
  ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Copy WASM test files to the directory: build/smart-test-compilation."
)

include(GoogleTest)
gtest_discover_tests(smart-test-compilation)
