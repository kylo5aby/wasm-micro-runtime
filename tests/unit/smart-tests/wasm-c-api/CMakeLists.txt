# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required (VERSION 3.14)
project (smart_tests_wasm_c_api)

################  runtime settings  ################
set(CMAKE_BUILD_TYPE Debug)
set(WAMR_BUILD_PLATFORM "linux")

# Resetdefault linker flags
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# WAMR features switch
# CRITICAL: These must be set BEFORE including runtime_lib.cmake
# because config_common.cmake (included by runtime_lib.cmake) checks these variables
if (NOT DEFINED WAMR_BUILD_TARGET)
  set(WAMR_BUILD_TARGET "X86_64")
endif()
# Ensure these are set as normal variables (not cache) before include
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_LIBC_BUILTIN 1)
set(WAMR_BUILD_LIBC_WASI 0)
set(WAMR_BUILD_FAST_INTERP 0)
set(WAMR_BUILD_DUMP_CALL_STACK 1)

# compiling and linking flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -pie -fPIE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wformat -Wformat-security -mindirect-branch-register --coverage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")

# build out vmlib
# hard code path here - need to go up 4 levels to reach wasm-micro-runtime root
set(WAMR_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../)
include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)

add_library(vmlib STATIC ${WAMR_RUNTIME_LIB_SOURCE})
################################################

################  unit test related  ################

# Yes. To solve the compatibility issue with CMAKE (>= 4.0), we need to update
# our `cmake_minimum_required()` to 3.5. However, there are CMakeLists.txt
# from 3rd parties that we should not alter. Therefore, in addition to
# changing the `cmake_minimum_required()`, we should also add a configuration
# here that is compatible with earlier versions.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.

if (NOT (GOOGLETEST_INCLUDED EQUAL 1))
# Prevent overriding the parent project's compiler/linker
# settings on Windows
set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Fetch Google test
include (FetchContent)
FetchContent_Declare (
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable (googletest)
endif()

enable_testing()

# Add common test helper include directory
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../common)

file(GLOB_RECURSE source_all ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

set(UNIT_SOURCE ${source_all})

add_executable(smart_tests_wasm_c_api ${UNIT_SOURCE})

target_link_libraries(smart_tests_wasm_c_api vmlib gtest_main)

include(GoogleTest)
gtest_discover_tests(smart_tests_wasm_c_api)
