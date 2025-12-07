# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.14)

project(smart-test-thread-mgr)

set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_FAST_INTERP 1)
set(WAMR_BUILD_THREAD_MGR 1)
set(WAMR_BUILD_LIB_PTHREAD 1)
set(WAMR_BUILD_SHARED_MEMORY 1)
set(WAMR_BUILD_PLATFORM "linux")
set(WAMR_BUILD_TARGET "X86_64")

# Include common unit test configuration
include(../../unit_common.cmake)

# Add thread-mgr enhanced coverage tests (revised naming)
set(unit_test_sources_step1
    thread_mgr_add_25_step_1.cc
    ${WAMR_RUNTIME_LIB_SOURCE}
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/thread-mgr/thread_manager.c
)

set(unit_test_sources_step2
    thread_mgr_add_25_step_2.cc
    ${WAMR_RUNTIME_LIB_SOURCE}
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/thread-mgr/thread_manager.c
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/lib-pthread/lib_pthread_wrapper.c
)

set(unit_test_sources_step3
    thread_mgr_add_25_step_3.cc
    ${WAMR_RUNTIME_LIB_SOURCE}
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/thread-mgr/thread_manager.c
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/lib-pthread/lib_pthread_wrapper.c
)

set(unit_test_sources_step4
    thread_mgr_add_25_step_4.cc
    ${WAMR_RUNTIME_LIB_SOURCE}
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/thread-mgr/thread_manager.c
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/lib-pthread/lib_pthread_wrapper.c
)

# Enhanced thread manager test sources for wasm_cluster_set_context coverage
set(unit_test_sources_enhanced
    enhanced_thread_manager_test.cc
    ${WAMR_RUNTIME_LIB_SOURCE}
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/thread-mgr/thread_manager.c
)

add_executable(thread_mgr_add_25_step_1 ${unit_test_sources_step1})
add_executable(thread_mgr_add_25_step_2 ${unit_test_sources_step2})
add_executable(thread_mgr_add_25_step_3 ${unit_test_sources_step3})
add_executable(thread_mgr_add_25_step_4 ${unit_test_sources_step4})
add_executable(enhanced_thread_manager_test ${unit_test_sources_enhanced})

target_link_libraries(thread_mgr_add_25_step_1 
    gtest_main 
    pthread
    ${LLVM_AVAILABLE_LIBS}
    ${UV_A_LIBS}
)

target_link_libraries(thread_mgr_add_25_step_2 
    gtest_main 
    pthread
    ${LLVM_AVAILABLE_LIBS}
    ${UV_A_LIBS}
)

target_link_libraries(thread_mgr_add_25_step_3 
    gtest_main 
    pthread
    ${LLVM_AVAILABLE_LIBS}
    ${UV_A_LIBS}
)

target_link_libraries(thread_mgr_add_25_step_4
    gtest_main
    pthread
    ${LLVM_AVAILABLE_LIBS}
    ${UV_A_LIBS}
)

target_link_libraries(enhanced_thread_manager_test
    gtest_main
    pthread
    ${LLVM_AVAILABLE_LIBS}
    ${UV_A_LIBS}
)

target_include_directories(thread_mgr_add_25_step_1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/thread-mgr
)

target_include_directories(thread_mgr_add_25_step_2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/thread-mgr
)

target_include_directories(thread_mgr_add_25_step_3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/thread-mgr
)

target_include_directories(thread_mgr_add_25_step_4 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/thread-mgr
)

target_include_directories(enhanced_thread_manager_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WAMR_ROOT_DIR}/core/iwasm/libraries/thread-mgr
)

# Enable thread manager compilation flags
target_compile_definitions(thread_mgr_add_25_step_1 PRIVATE
    WASM_ENABLE_THREAD_MGR=1
    WASM_ENABLE_INTERP=1
)

target_compile_definitions(thread_mgr_add_25_step_2 PRIVATE
    WASM_ENABLE_THREAD_MGR=1
    WASM_ENABLE_INTERP=1
    WASM_ENABLE_LIB_PTHREAD=1
    WASM_ENABLE_SHARED_MEMORY=1
)

target_compile_definitions(thread_mgr_add_25_step_3 PRIVATE
    WASM_ENABLE_THREAD_MGR=1
    WASM_ENABLE_INTERP=1
    WASM_ENABLE_LIB_PTHREAD=1
    WASM_ENABLE_SHARED_MEMORY=1
)

target_compile_definitions(thread_mgr_add_25_step_4 PRIVATE
    WASM_ENABLE_THREAD_MGR=1
    WASM_ENABLE_INTERP=1
    WASM_ENABLE_LIB_PTHREAD=1
    WASM_ENABLE_SHARED_MEMORY=1
)

# Enhanced test with MODULE_INST_CONTEXT support for wasm_cluster_set_context coverage
target_compile_definitions(enhanced_thread_manager_test PRIVATE
    WASM_ENABLE_THREAD_MGR=1
    WASM_ENABLE_INTERP=1
    WASM_ENABLE_MODULE_INST_CONTEXT=1
)

# Add tests to CTest
add_test(NAME thread_mgr_add_25_step_1 COMMAND thread_mgr_add_25_step_1)
add_test(NAME thread_mgr_add_25_step_2 COMMAND thread_mgr_add_25_step_2)
add_test(NAME thread_mgr_add_25_step_3 COMMAND thread_mgr_add_25_step_3)
add_test(NAME thread_mgr_add_25_step_4 COMMAND thread_mgr_add_25_step_4)
add_test(NAME enhanced_thread_manager_test COMMAND enhanced_thread_manager_test)