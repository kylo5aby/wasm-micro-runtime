# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.14)
project(wasm-apps)

# Copy pre-compiled WASM test files to parent directory for test execution
add_custom_target(copy_wasm_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/i32_add_test.wasm
        ${CMAKE_CURRENT_BINARY_DIR}/../i32_add_test.wasm
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/i32_add_test.wat
        ${CMAKE_CURRENT_BINARY_DIR}/../i32_add_test.wat
    COMMENT "Copy i32_add_test WASM files to the same directory as google test"
)