# CMakeLists.txt for SIMD extension opcode unit tests
# Builds comprehensive test suite for SIMD opcodes including v128.load, i8x16.splat, i16x8.splat, i32x4.splat, i64x2.splat, f64x2.splat, i8x16.lt_s, i8x16.lt_u, f64x2.extmul_high_i32x4_s, f64x2.extmul_high_i32x4_u, i16x8.extmul_high_i8x16_u
# with SIMD extension support and cross-execution mode validation

cmake_minimum_required(VERSION 3.14)

project(smart-test-extension-opcodes)

# Enable required WAMR build features for SIMD opcode testing (v128.load, i8x16.splat, i16x8.splat, i32x4.splat, i64x2.splat, f64x2.splat, i8x16.lt_s, i8x16.lt_u, f64x2.extmul_high_i32x4_s, f64x2.extmul_high_i32x4_u, i16x8.extmul_high_i8x16_u)
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

set(UNIT_SOURCE ${source_all})

# Prepare unit test sources
set(unit_test_sources
        ${UNIT_SOURCE}
        ${WAMR_RUNTIME_LIB_SOURCE}
        ${UNCOMMON_SHARED_SOURCE}
        )

# Create test executables for each SIMD opcode
add_executable(enhanced_v128_load_test enhanced_v128_load_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
add_executable(enhanced_i8x16_splat_test enhanced_i8x16_splat_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
add_executable(enhanced_i16x8_splat_test enhanced_i16x8_splat_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
add_executable(enhanced_i32x4_splat_test enhanced_i32x4_splat_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
add_executable(enhanced_i64x2_splat_test enhanced_i64x2_splat_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
add_executable(enhanced_f64x2_splat_test enhanced_f64x2_splat_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
add_executable(enhanced_i8x16_lt_s_test enhanced_i8x16_lt_s_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
add_executable(enhanced_i8x16_lt_u_test enhanced_i8x16_lt_u_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
add_executable(enhanced_f64x2_extmul_high_i32x4_s_test enhanced_f64x2_extmul_high_i32x4_s_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
add_executable(enhanced_f64x2_extmul_high_i32x4_u_test enhanced_f64x2_extmul_high_i32x4_u_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
add_executable(enhanced_i16x8_extmul_high_i8x16_u_test enhanced_i16x8_extmul_high_i8x16_u_test.cc ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})

# Link required libraries
target_link_libraries(enhanced_v128_load_test ${LLVM_AVAILABLE_LIBS} gtest_main)
target_link_libraries(enhanced_i8x16_splat_test ${LLVM_AVAILABLE_LIBS} gtest_main)
target_link_libraries(enhanced_i16x8_splat_test ${LLVM_AVAILABLE_LIBS} gtest_main)
target_link_libraries(enhanced_i32x4_splat_test ${LLVM_AVAILABLE_LIBS} gtest_main)
target_link_libraries(enhanced_i64x2_splat_test ${LLVM_AVAILABLE_LIBS} gtest_main)
target_link_libraries(enhanced_f64x2_splat_test ${LLVM_AVAILABLE_LIBS} gtest_main)
target_link_libraries(enhanced_i8x16_lt_s_test ${LLVM_AVAILABLE_LIBS} gtest_main)
target_link_libraries(enhanced_i8x16_lt_u_test ${LLVM_AVAILABLE_LIBS} gtest_main)
target_link_libraries(enhanced_f64x2_extmul_high_i32x4_s_test ${LLVM_AVAILABLE_LIBS} gtest_main)
target_link_libraries(enhanced_f64x2_extmul_high_i32x4_u_test ${LLVM_AVAILABLE_LIBS} gtest_main)
target_link_libraries(enhanced_i16x8_extmul_high_i8x16_u_test ${LLVM_AVAILABLE_LIBS} gtest_main)

# Enable test discovery
include(GoogleTest)
gtest_discover_tests(enhanced_v128_load_test)
gtest_discover_tests(enhanced_i8x16_splat_test)
gtest_discover_tests(enhanced_i16x8_splat_test)
gtest_discover_tests(enhanced_i32x4_splat_test)
gtest_discover_tests(enhanced_i64x2_splat_test)
gtest_discover_tests(enhanced_f64x2_splat_test)
gtest_discover_tests(enhanced_i8x16_lt_s_test)
gtest_discover_tests(enhanced_i8x16_lt_u_test)
gtest_discover_tests(enhanced_f64x2_extmul_high_i32x4_s_test)
gtest_discover_tests(enhanced_f64x2_extmul_high_i32x4_u_test)
gtest_discover_tests(enhanced_i16x8_extmul_high_i8x16_u_test)

# Create a custom target to copy WASM test files once for all tests
add_custom_target(copy_extension_wasm_apps ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/wasm-apps
    ${CMAKE_CURRENT_BINARY_DIR}/wasm-apps
    COMMENT "Copying SIMD WASM test files to build directory"
)

# Make all test executables depend on the copy operation
add_dependencies(enhanced_v128_load_test copy_extension_wasm_apps)
add_dependencies(enhanced_i8x16_splat_test copy_extension_wasm_apps)
add_dependencies(enhanced_i16x8_splat_test copy_extension_wasm_apps)
add_dependencies(enhanced_i32x4_splat_test copy_extension_wasm_apps)
add_dependencies(enhanced_i64x2_splat_test copy_extension_wasm_apps)
add_dependencies(enhanced_f64x2_splat_test copy_extension_wasm_apps)
add_dependencies(enhanced_i8x16_lt_s_test copy_extension_wasm_apps)
add_dependencies(enhanced_i8x16_lt_u_test copy_extension_wasm_apps)
add_dependencies(enhanced_f64x2_extmul_high_i32x4_s_test copy_extension_wasm_apps)
add_dependencies(enhanced_f64x2_extmul_high_i32x4_u_test copy_extension_wasm_apps)
add_dependencies(enhanced_i16x8_extmul_high_i8x16_u_test copy_extension_wasm_apps)