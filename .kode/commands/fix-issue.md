---
name: fix-issue
description: Apply systematic issue resolution protocol for WAMR unit test module
aliases: [fix, debug-test, resolve]
argNames: [module]
progressMessage: Applying issue resolution protocol for {module} module...
---

Apply the comprehensive WAMR Unit Test Issue Resolution Protocol to diagnose and fix build failures, compilation errors, and test execution issues for the **{module}** module.

## Protocol Execution for {module}

### Phase 1: Pre-Fix Analysis and Validation

**Issue Classification for {module}:**
```bash
# Navigate to unit test directory
cd tests/unit/

# Identify issue type for {module}
cmake -S . -B build -DCOLLECT_CODE_COVERAGE=1 2>&1 | tee build.log
cmake --build build 2>&1 | tee compile.log
ctest --test-dir build 2>&1 | tee test.log

# Focus on {module} specific issues
grep -i "{module}" build.log compile.log test.log
```

**Reference Working Examples:**
- Examine `tests/unit/aot/CMakeLists.txt`
- Study `tests/unit/shared-utils/CMakeLists.txt` 
- Review `tests/unit/interpreter/CMakeLists.txt`
- Check `tests/unit/memory64/CMakeLists.txt`

**Feature Context for {module}:**
- Read source code in `core/iwasm/{module}/`
- Understand {module} feature specifications and edge cases
- Identify integration points with other WAMR features
- Review existing {module} related tests for patterns

### Phase 2: Build Configuration Issues

**CMakeLists.txt Template for {module}:**
```cmake
cmake_minimum_required(VERSION 3.0)
project(test_{module})

include(../unit_common.cmake)
include_directories(${{CMAKE_CURRENT_SOURCE_DIR}})

# Use GLOB not GLOB_RECURSE for {module}
file(GLOB source_all ${{CMAKE_CURRENT_SOURCE_DIR}}/*.cc)

set(unit_test_sources
    ${{UNIT_SOURCE}}
    ${{WAMR_RUNTIME_LIB_SOURCE}}
    ${{UNCOMMON_SHARED_SOURCE}}
    # DO NOT add ${{SRC_LIST}} or aux_source_directory
)

set(UNIT_SOURCE ${{source_all}})
set(UNIT_SOURCE ${{UNIT_SOURCE}} ${{unit_test_sources}})

add_executable(test_{module} ${{UNIT_SOURCE}})
target_link_libraries(test_{module} ${{LLVM_AVAILABLE_LIBS}} gtest_main)
gtest_discover_tests(test_{module})
```

### Phase 3: Compilation Error Resolution

**Standard Includes for {module} Tests:**
```cpp
#include "gtest/gtest.h"
#include "test_helper.h"
#include "wasm_export.h"
#include "bh_read_file.h"
#include "wasm_runtime_common.h"

// {module}-specific includes
#if WASM_ENABLE_AOT != 0
#include "aot_runtime.h"
#endif

#if WASM_ENABLE_INTERP != 0
#include "wasm_runtime.h"
#endif
```

### Phase 4: Test Execution Failure Resolution

**Debug {module} Tests:**
```bash
# Run {module} specific tests
./build/test_{module}/test_{module} --gtest_filter="*{module}*"

# Debug failing {module} tests
gdb ./build/test_{module}/test_{module}
(gdb) set args --gtest_filter="*FailingTest*"
(gdb) run
(gdb) bt

# Memory check for {module}
valgrind --tool=memcheck ./build/test_{module}/test_{module}
```

**High-Quality Test Pattern for {module}:**
```cpp
class {module}Test : public testing::Test {{
protected:
    void SetUp() override {{
        // Initialize WAMR runtime for {module} testing
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        
        ASSERT_TRUE(wasm_runtime_full_init(&init_args));
        
        // Setup {module}-specific resources
        test_resource = setup_{module}_test_resource();
        ASSERT_NE(nullptr, test_resource) << "Failed to setup {module} test resource";
    }}
    
    void TearDown() override {{
        // Cleanup {module} resources
        if (test_resource) {{
            cleanup_{module}_resource(test_resource);
            test_resource = nullptr;
        }}
        
        wasm_runtime_destroy();
    }}
    
private:
    void* test_resource = nullptr;
}};

TEST_F({module}Test, Test{module}CoreFunctionality) {{
    // Test {module} specific feature
    {module}Result result = execute_{module}_operation(test_resource);
    
    // Use ASSERT for definitive validation
    ASSERT_EQ({module}_SUCCESS, result.status) << "{module} operation failed";
    ASSERT_GT(result.value, 0) << "{module} returned invalid value";
    ASSERT_LE(result.value, MAX_{module}_VALUE) << "{module} value exceeds maximum";
}}
```

### Phase 5: Iterative Fix Process

**One-Issue-at-a-Time for {module}:**
```bash
# Step 1: Identify first failing {module} test
./build/test_{module}/test_{module} --gtest_list_tests | grep FAILED

# Step 2: Fix only that test
./build/test_{module}/test_{module} --gtest_filter="*FirstFailingTest*"

# Step 3: Verify fix doesn't break other {module} tests
./build/test_{module}/test_{module} --gtest_filter="*{module}*"
```

**Fix Attempt Limit for {module}:**
1. **Attempt 1**: Basic {module} assertion and logic fixes
2. **Attempt 2**: {module} resource initialization and cleanup fixes
3. **Attempt 3**: {module} platform-specific conditional logic
4. **Attempt 4**: Deep {module} feature behavior analysis and correction
5. **Attempt 5**: Final {module} debugging attempt

**After 5 failed attempts**: Remove problematic {module} test code and document.

### Phase 6: Quality Validation

**{module} Test Quality Checklist:**
- [ ] Test validates actual {module} WAMR feature functionality
- [ ] Uses ASSERT_* for definitive pass/fail semantics
- [ ] Tests both {module} success and error paths
- [ ] Covers {module} edge cases and boundaries
- [ ] Proper {module} resource cleanup maintained
- [ ] Handles {module} platform differences gracefully
- [ ] Tests {module} integration with other WAMR features

**Final {module} Validation:**
```bash
cd tests/unit/

# Clean build for {module}
rm -rf build/
cmake -S . -B build -DCOLLECT_CODE_COVERAGE=1

# Successful {module} compilation
cmake --build build
echo "{module} build exit code: $?"

# All {module} tests pass
ctest --test-dir build --output-on-failure -R {module}
echo "{module} test exit code: $?"
```

### Success Criteria for {module}

**Mandatory Success Indicators:**
- [ ] All {module} tests compile without errors
- [ ] All {module} test cases pass when executed
- [ ] {module} tests use ASSERT_* for clear pass/fail semantics
- [ ] No memory leaks or crashes in {module} tests
- [ ] Proper {module} resource initialization and cleanup
- [ ] {module} tests validate actual WAMR feature functionality
- [ ] Both positive and negative {module} test scenarios covered
- [ ] {module} platform differences handled gracefully

## Critical Boundaries for {module}

### ❌ FORBIDDEN for {module}:
- NEVER modify committed {module} source code files
- NEVER change existing {module} WASM binary files
- NEVER alter existing {module} test logic in committed files
- NEVER modify core {module} WAMR runtime implementation

### ✅ PERMITTED for {module}:
- Create new {module} test files: `test_{module}_[feature].cc`
- Modify {module} `CMakeLists.txt` in unit test directories
- Generate new {module} WAT files and WASM binaries for tests
- Add new {module} helper utilities in test directories

## Quick Reference for {module}

**Build Fixes:**
```cmake
# Use GLOB not GLOB_RECURSE for {module}
file(GLOB source_all ${{CMAKE_CURRENT_SOURCE_DIR}}/*.cc)
```

**Test Logic Fixes:**
```cpp
// Use ASSERT not EXPECT for {module}
ASSERT_EQ(expected, actual);
ASSERT_TRUE(condition);
```

**Resource Management:**
```cpp
void SetUp() override {{
    ASSERT_TRUE(wasm_runtime_full_init(&init_args));
}}

void TearDown() override {{
    wasm_runtime_destroy();
}}
```

Apply this systematic protocol to resolve all {module} unit test issues while maintaining strict boundaries and ensuring high-quality test validation.