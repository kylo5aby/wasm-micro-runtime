---
name: cross-module-test
description: Generate tests for WAMR macro-controlled code by relocating to compatible modules with required build flags
version: "1.0.0"
---

# Cross-Module Test Generation for Macro-Controlled Code

## Skill Description
This skill handles test generation for WAMR code that is controlled by compile-time macros but requires different build flags than those available in the current target module. It automatically identifies macro-controlled code blocks and relocates test generation to compatible modules that have the required build flags enabled.

## When to Invoke This Skill
- **Macro-Controlled Code Detection**: When encountering code blocks wrapped in `#if WASM_ENABLE_*` conditions
- **Build Flag Incompatibility**: When the current module's CMakeLists.txt has required flags disabled (set to 0)
- **Cross-Module Dependencies**: When code in one module requires features enabled in another module for testing
- **Coverage Gaps**: When standard test generation fails due to macro-controlled code paths

## Identification Triggers
Invoke this skill when you encounter any of these patterns:

```c
#if WASM_ENABLE_AOT != 0
    // AOT-specific code that needs testing
#endif

#if WASM_ENABLE_GC != 0
    // Garbage collection code
#endif

#if WASM_ENABLE_SHARED_HEAP != 0
    // Shared heap functionality
#endif

#if WASM_ENABLE_MEMORY64 != 0
    // 64-bit memory addressing code
#endif

#if WASM_ENABLE_THREAD_MGR != 0
    // Threading management code
#endif
```

---

## Cross-Module Test Generation Protocol

### Step 1: Macro-Controlled Code Detection

**MANDATORY ANALYSIS**: Scan the target source code for conditional compilation blocks.

**Common WAMR Macro Patterns:**
- `#if WASM_ENABLE_AOT != 0` - AOT compilation features
- `#if WASM_ENABLE_GC != 0` - Garbage collection features
- `#if WASM_ENABLE_SHARED_HEAP != 0` - Shared heap functionality
- `#if WASM_ENABLE_MEMORY64 != 0` - 64-bit memory addressing
- `#if WASM_ENABLE_THREAD_MGR != 0` - Threading management
- `#if WASM_ENABLE_JIT != 0` - Just-In-Time compilation
- `#if WASM_ENABLE_FAST_JIT != 0` - Fast JIT compilation
- `#if WASM_ENABLE_SIMD != 0` - SIMD operations
- `#if WASM_ENABLE_REF_TYPES != 0` - Reference types
- `#if WASM_ENABLE_BULK_MEMORY != 0` - Bulk memory operations

### Step 2: Build Flag Compatibility Check

**MANDATORY VERIFICATION PROCESS:**

1. **Current Module Analysis**: Examine `tests/unit/[current_module]/CMakeLists.txt`
2. **Flag Status Check**: Determine if required build flags are enabled (set to 1)
3. **Compatibility Assessment**: Identify flag mismatches requiring relocation

**Build Flag to Module Mapping Reference:**

| Macro Condition | Required CMake Flag | Compatible Test Modules |
|----------------|-------------------|---------------------|
| `WASM_ENABLE_AOT != 0` | `WAMR_BUILD_AOT=1` | `aot/`, `compilation/`, `aot-stack-frame/` |
| `WASM_ENABLE_GC != 0` | `WAMR_BUILD_GC=1` | `gc/` |
| `WASM_ENABLE_SHARED_HEAP != 0` | `WAMR_BUILD_SHARED_HEAP=1` | `shared-heap/` |
| `WASM_ENABLE_MEMORY64 != 0` | `WAMR_BUILD_MEMORY64=1` | `memory64/`, `shared-heap/` |
| `WASM_ENABLE_THREAD_MGR != 0` | `WAMR_BUILD_THREAD_MGR=1` | `interpreter/`, `compilation/`, `runtime-common/` |
| `WASM_ENABLE_JIT != 0` | `WAMR_BUILD_JIT=1` | `running-modes/` |
| `WASM_ENABLE_FAST_JIT != 0` | `WAMR_BUILD_FAST_JIT=1` | `running-modes/` |
| `WASM_ENABLE_SIMD != 0` | `WAMR_BUILD_SIMD=1` | `aot-stack-frame/` |
| `WASM_ENABLE_REF_TYPES != 0` | `WAMR_BUILD_REF_TYPES=1` | `aot-stack-frame/`, `runtime-common/` |
| `WASM_ENABLE_BULK_MEMORY != 0` | `WAMR_BUILD_BULK_MEMORY=1` | `interpreter/` |

### Step 3: Target Module Selection

**Selection Priority Criteria (Highest to Lowest):**

1. **Feature-Specific Module**: Dedicated module for the feature (e.g., `aot/` for AOT code)
2. **Multi-Feature Module**: Module enabling multiple related features (e.g., `compilation/` for AOT+threading)
3. **Integration Module**: Module testing feature interactions (e.g., `runtime-common/` for cross-cutting features)

**Selection Algorithm:**
```bash
# 1. Identify required macro and corresponding flag
MACRO_CONDITION="WASM_ENABLE_AOT"
REQUIRED_FLAG="WAMR_BUILD_AOT=1"

# 2. Find compatible modules from mapping table
COMPATIBLE_MODULES=("aot" "compilation" "aot-stack-frame")

# 3. Select highest priority module that exists and has flag enabled
TARGET_MODULE=$(select_highest_priority_module "${COMPATIBLE_MODULES[@]}")
```

### Step 4: Cross-Module Test Generation

**Test Relocation Protocol:**

1. **Target Module Identification**: Select optimal module using selection criteria
2. **Source File Mapping**: Maintain reference to original source location
3. **Cross-Module File Naming**: Use special naming convention for relocated tests
4. **Enhanced Documentation**: Include detailed cross-reference information

**Cross-Module Test Template:**
```cpp
/******
 * CROSS-MODULE TEST CASE
 * Test Case: [OriginalModule]_[FunctionName]_[Scenario]_[ExpectedOutcome]
 * Original Source: core/iwasm/[original_module]/[source_file].c:[line_numbers]
 * Target Module: [target_module] (relocated due to [REQUIRED_FLAG] requirement)
 * Macro Condition: #if [MACRO_CONDITION] != 0
 * Relocation Reason: Original module ([original_module]) has [REQUIRED_FLAG]=0
 * Functional Purpose: [Description of what the code does]
 * Coverage Goal: Exercise [macro_condition]-specific path in [original_module] code
 ******/
TEST_F(Enhanced[TargetModule]Test, [OriginalModule]_[FunctionName]_[Scenario]_[ExpectedOutcome]) {
    // CROSS-MODULE TEST NOTICE
    // Original Module: [original_module]
    // Target Module: [target_module]
    // Relocation Reason: Required flag [REQUIRED_FLAG] not enabled in original module
    // Original Source: [source_file_path]:[line_numbers]

    // Test implementation for [original_module] code in [target_module] context
    // ... test code here ...

    ASSERT_TRUE(test_validates_macro_controlled_functionality());
}
```

### Step 5: Cross-Module File Organization

**File Naming Conventions:**

- **Cross-Module Test File**: `tests/unit/[target_module]/enhanced_[original_source_file]_cross_test.cc`
- **Test Fixture Class**: `Enhanced[OriginalSourceFile]CrossTest`

**Examples:**
- For `wasm_runtime_common.c` AOT code → `tests/unit/aot/enhanced_wasm_runtime_common_cross_test.cc`
- For `wasm_loader.c` GC code → `tests/unit/gc/enhanced_wasm_loader_cross_test.cc`
- For `wasm_interp.c` SIMD code → `tests/unit/aot-stack-frame/enhanced_wasm_interp_cross_test.cc`

**CMakeLists.txt Integration Steps:**

1. **Add Cross-Module Test File**: Include the new `*_cross_test.cc` file in target module's CMakeLists.txt
2. **Include Original Headers**: Add include paths for original module if needed
3. **Verify Build Flags**: Ensure all required flags are enabled in target module
4. **Dependency Resolution**: Add any additional libraries or dependencies required

### Step 6: Documentation and Validation

**MANDATORY Cross-Reference Documentation Block:**
```cpp
// ===================================================================
// CROSS-MODULE TEST DOCUMENTATION
// ===================================================================
// Original Module: [original_module_name]
// Target Module: [target_module_name]
// Relocation Trigger: Macro condition [MACRO_CONDITION] != 0
// Required Flag: [REQUIRED_FLAG]=1 (disabled in original module)
// Original Source File: core/iwasm/[original_module]/[source_file].c
// Target Lines: [line_numbers]
// Relocation Date: [YYYY-MM-DD]
// ===================================================================
```

**Build and Test Validation Protocol:**

1. **Cross-Module Build Test**: Verify target module builds successfully with cross-module tests
2. **Runtime Validation**: Execute cross-module tests and verify they pass
3. **Coverage Attribution**: Ensure coverage metrics are attributed to original source file
4. **Integration Verification**: Confirm cross-module tests don't interfere with existing tests

## Enforcement Requirements

**CRITICAL COMPLIANCE RULES:**

✅ **MUST DO:**
1. **Flag Verification**: Always verify build flag compatibility before generating tests
2. **Module Relocation**: Relocate to appropriate module when flags are incompatible
3. **Cross-Reference Documentation**: Document relocation reason and original source
4. **Build Validation**: Verify target module can build and execute cross-module tests
5. **Coverage Attribution**: Attribute coverage to original source file, not target module

❌ **ABSOLUTE PROHIBITIONS:**
1. **Ignoring Macro Conditions**: Never generate tests for macro-disabled code in incompatible modules
2. **Flag Override Attempts**: Never modify CMakeLists.txt to enable incompatible flags in original module
3. **Undocumented Relocation**: Never relocate tests without proper cross-reference documentation
4. **Silent Failures**: Never skip macro-controlled code without attempting cross-module relocation
5. **Coverage Misattribution**: Never attribute cross-module test coverage to target module instead of original source

## Success Criteria

**Completion Checklist:**
- [ ] Macro-controlled code blocks identified and analyzed
- [ ] Build flag compatibility verified for current and target modules
- [ ] Appropriate target module selected using priority criteria
- [ ] Cross-module test file created with proper naming convention
- [ ] Test cases include comprehensive cross-reference documentation
- [ ] CMakeLists.txt updated in target module if necessary
- [ ] Cross-module tests build successfully in target module
- [ ] All cross-module tests pass execution
- [ ] Coverage metrics correctly attributed to original source file
- [ ] Cross-module integration validated without conflicts

## Usage Example

**Invocation Scenario:**
```
Original Issue: Code in core/iwasm/common/wasm_runtime_common.c lines 1234-1250
                controlled by #if WASM_ENABLE_AOT != 0 cannot be tested because
                tests/unit/runtime-common/CMakeLists.txt has WAMR_BUILD_AOT=0

Skill Response:
1. Detects WASM_ENABLE_AOT macro condition
2. Identifies WAMR_BUILD_AOT=1 requirement
3. Finds runtime-common module has WAMR_BUILD_AOT=0 (incompatible)
4. Selects aot/ module as highest priority compatible target
5. Creates tests/unit/aot/enhanced_wasm_runtime_common_cross_test.cc
6. Generates cross-module tests with proper documentation
7. Validates build and execution in aot module context
8. Attributes coverage to original wasm_runtime_common.c source
```

This skill ensures comprehensive test coverage for macro-controlled WAMR code by intelligently relocating test generation to compatible modules while maintaining full traceability and documentation.