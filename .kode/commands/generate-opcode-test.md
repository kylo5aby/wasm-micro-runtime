# WASM Opcode Test Enhancement Agent

Generate comprehensive, production-ready WASM opcode test suites using a systematic 6-phase approach with mandatory TODO list management.

## Mission Statement
Create complete test coverage for WASM opcodes through ultra-deep analysis, strategic test planning, robust code generation, thorough validation, issue resolution, and standardized commits. Every test case must validate actual WAMR runtime behavior with comprehensive assertions.

## Input Requirements
**OPCODE_NAME**: Target WASM opcode (e.g., i32.add, br_if, f64.mul, memory.grow)

## Mandatory Output Deliverables
1. **Enhanced Opcode Test File**: `enhanced_[OPCODE_NAME]_test.cc`, `wat/wasm`(if has) - e.g., `enhanced_i64_clz_test.cc` for opcode i64_clz
2. **CMakeLists.txt**: If integration is required
3. **Git Commit**: Properly formatted commit with standardized message
---

## 🔐 MANDATORY TODO List Protocol

**CRITICAL REQUIREMENT**: You MUST create and maintain a structured TODO list before initiating any work.

**Operational Flow**:
1. Create TODO List → 2. Execute Current Task → 3. Update TODO List → 4. Repeat until completion

### Standardized TODO List Template
```markdown
## WASM Opcode Test Generation TODO List

### Phase 1: Ultra-Deep Opcode Analysis
- [ ] 1.1 Semantic analysis of opcode functionality and purpose
- [ ] 1.2 Type system analysis (input/output types, conversions, polymorphism)
- [ ] 1.3 Opcode category classification (numeric, memory, control-flow, variable, reference, simd, reference, table...)

### Phase 2: Strategic Test Planning
- [ ] 2.1 Main routine test case design (basic functionality with typical values)
- [ ] 2.2 Cross-execution mode validation strategy planning
- [ ] 2.3 Detailed test case descriptions and expected outcomes specification

### Phase 3: Complete Code Generation
- [ ] 3.1 Directory structure creation in tests/unit/enhanced_opcode/{CATEGORY}/
- [ ] 3.2 enhanced_{opcode}_test.cc generation with GTest framework
- [ ] 3.3 {opcode}_common.h header file generation (if needed)
- [ ] 3.4 Comprehensive WASM test files generation ({opcode}_test.wat and .wasm)
- [ ] 3.5 CMakeLists.txt generation with proper dependencies
- [ ] 3.6 ALL test cases implementation with meaningful ASSERT_* statements (NO GTEST_SKIP/SUCCEED/FAIL)

### Phase 4: Build & Test Execution
- [ ] 4.1 Navigate to tests/unit/ and configure CMake build
- [ ] 4.2 Build test suite with parallel compilation
- [ ] 4.3 Execute test suite and capture detailed output
- [ ] 4.4 Validate all tests pass (0 failures) and no runtime crashes(crash like segment fault/core dump, etc)
- [ ] 4.5 Execution validation
- [ ] 4.6 Document any build/test failures for Phase 5 resolution

### Phase 5: Issue Detection & Resolution (Conditional - Only if Phase 4 fails)
- [ ] 5.1 Comprehensive issue detection and analysis
- [ ] 5.2 Root cause categorization (compilation, runtime, test logic, crashes)
- [ ] 5.3 Targeted resolution application (preserving test intention)
- [ ] 5.4 Verification & iteration control (max 5 cycles through 5.1-5.4)
- [ ] 5.5 Final resolution decision (evaluate after max iterations)
- [ ] 5.6 Task failure cleanup (only if task must be terminated)

### Phase 6: Code Review & Standardized Commit
- [ ] 6.1 Comprehensive code quality review
- [ ] 6.2 Files staging and standardized commit message creation
- [ ] 6.3 Commit execution and repository state validation
```

### TODO Update Protocol
After EVERY task completion:
1. Mark completed tasks with ✅ checkbox
2. Update current progress status with explicit percentage
3. Display the updated TODO list in its entirety
4. Explicitly declare the next task to be executed

---
## 🚨 Enforcement Policies

### ABSOLUTE REQUIREMENTS
**Project Management:**  
1. **TODO List Creation**: Create structured TODO list before initiating any work  
2. **Phase Sequential Execution**: Complete phases in strict order (1→2→3→4→(5 if needed)→6)  
3. **Progress Tracking**: Update TODO list after every task completion  

**Code Quality Standards:**  

4. **No Test Skipping**: NEVER use GTEST_SKIP(), SUCCEED(), or FAIL()  
5. **Functional Validation**: Validate actual WAMR runtime behavior, not just code execution  
6. **Meaningful Assertions**: Include substantial assertions in every test case  
7. **Naming Conventions**: Follow `TEST_F({opcode}_test_suite, Function_Scenario_ExpectedOutcome)` pattern  
8. **Resource Management**: Implement proper SetUp/TearDown with RAII patterns  

**Build and Testing:**  

9. **Build Location**: Build exclusively in `tests/unit/` directory  
10. **Directory Structure**: Create files in `tests/unit/enhanced_opcode/{CATEGORY}/` following exact patterns  
11. **Test Completeness**: Implement ALL test cases from Phase 2 strategy  
12. **Build Success**: Achieve zero compilation errors and warnings  
13. **Test Success**: Achieve 100% test pass rate (zero failures)  
14. **Commit Compliance**: Use EXACT commit message template  

### ABSOLUTE PROHIBITIONS

**Workflow Violations:**
1. Starting work without creating TODO list  
2. Bypassing any required phase or task within phases  
3. Skipping iterative issue resolution cycles  

**Invalid Code Constructs:**  

4. Using GTEST_SKIP(), placeholder assertions, or non-substantive validations  
5. Using trivial assertions: ASSERT(true), ASSERT_TRUE(true), ASSERT_FALSE(false)  
6. Using if conditions without ASSERT validation for operation results  
7. Building tests outside tests/unit/ directory  

**Quality Compromises:**  

8. Generating partial test suites or missing test categories  
9. Accepting compilation warnings, test failures, or runtime crashes  
10. Missing function comments with source location
11. Adding content beyond specified commit message template  

## 🚨 ABSOLUTE COMPLIANCE REQUIREMENTS

**MANDATORY EXECUTION ORDER - NO EXCEPTIONS PERMITTED:**

### STRICT ADHERENCE RULES (NON-NEGOTIABLE):
1. **SEQUENTIAL EXECUTION ONLY**: Execute phases in EXACT order: 1→2→3→4→(5 if needed)→6
2. **NO STEP SKIPPING**: Complete EVERY sub-step within each phase before proceeding
3. **NO SHORTCUTS**: Follow ALL validation criteria and checkpoints
4. **NO IMPROVISATION**: Do not deviate from prescribed methods or add unlisted steps
5. **MANDATORY TODO TRACKING**: Update TODO list after EVERY single step completion

### ZERO TOLERANCE VIOLATIONS:
- ❌ Starting any phase without completing the previous phase entirely
- ❌ Skipping any numbered sub-step within a phase
- ❌ Proceeding without proper validation at each checkpoint
- ❌ Bypassing TODO list updates after step completion
- ❌ Adding custom steps or modifications to the prescribed workflow

### ENFORCEMENT PROTOCOL:
- **IMMEDIATE FAILURE**: Any deviation from prescribed steps results in IMMEDIATE task failure
- **NO RECOVERY**: Violations cannot be corrected - task must be restarted from Phase 1
- **STRICT VALIDATION**: Each step must meet ALL specified criteria before proceeding

---
## Systematic Workflow Execution

### PHASE 1: Ultra-Deep Opcode Analysis

Perform comprehensive analysis of the target opcode through sequential steps:

#### Step 1.1: Semantic Analysis
- **Research opcode specification**
- **Identify primary function**: What does this opcode do?
- **Document operation purpose**: Why does this opcode exist?
- **Map to runtime behavior**: How does WAMR implement this operation?

#### Step 1.2: Type System Analysis
- **Input types identification**: What types does the opcode consume from stack?
- **Output types determination**: What types does the opcode produce on stack?
- **Type conversion rules**: Any implicit conversions or validations?
- **Polymorphism handling**: Does the opcode work with multiple types?

#### Step 1.3: Category Classification
Classify into one of these categories based on analysis:  
For example:
- **Numeric**: `i32.add`, `f64.mul`, `i64.eqz` - Arithmetic, comparison, bitwise operations
- **Memory**: `i32.load`, `memory.grow` - Load, store, memory control operations
- **Control Flow**: `br_if`, `call`, `loop` - Branching, loops, function calls
- **Variable**: `local.get`, `global.set` - Local and global variable access
- **Reference**: `ref.null`, `ref.func` - Reference type operations
- **Extension**: `v128.add`, `i32.atomic.load` - SIMD, atomic operations

### PHASE 2: Strategic Test Planning
Generate comprehensive test strategy through systematic planning steps  
**Note**: Not every opcode has the need to contain all below case situations

#### Step 2.1: Main Routine Test Case Design
Design tests for basic opcode functionality:
- **Typical input scenarios**: Common, expected use cases
- **Standard value ranges**: Normal operational parameters
- **Positive test cases**: Verify correct behavior under normal conditions
- **Cross-execution mode validation**: Ensure consistency between interpreter and AOT modes
- **Expected success outcomes**: Document anticipated results for valid inputs

#### Step 2.2: Cross-Execution Mode Validation Strategy Planning
Plan validation across WAMR execution modes:
- **Interpreter mode testing**: Direct bytecode interpretation validation
- **AOT mode testing**: Ahead-of-time compiled module validation
- **Consistency verification**: Ensure identical results across modes

#### Step 2.3: Detailed Test Case Specifications
For each test category, document comprehensive specifications:
- **Test case descriptions**: Clear, specific test scenario explanations
- **Input conditions**: Exact parameter values and setup requirements
- **Expected outcomes**: Precise anticipated results and behaviors
- **Assertion statements**: Meaningful ASSERT_* validations with descriptive messages
- **WASM module requirements**: Module structure, exports, and dependencies

**Test Naming Guidelines:**
- Use broader, category-based names (e.g., `BasicAddition_ReturnsCorrectSum`)
- Focus on validation concept rather than specific input variations
- Each method validates one primary concept through multiple assertions
- Follow pattern: `TEST_P(OpcodeTest, Concept_ExpectedOutcome)`

**Consolidated Test Case Design Strategy**  
Group related scenarios to reduce method proliferation:

**Implementation Example:**
```cpp
// ✅ RECOMMENDED: Consolidated approach
TEST_P(I32AddTest, BasicAddition_ReturnsCorrectSum) {
    ASSERT_EQ(call_i32_add(5, 3), 8);        // Positive numbers
    ASSERT_EQ(call_i32_add(-10, -15), -25);  // Negative numbers
    ASSERT_EQ(call_i32_add(20, -8), 12);     // Mixed signs
}

// ❌ AVOID: Separate methods for similar scenarios
TEST_P(I32AddTest, BasicAddition_SmallPositives_ReturnsCorrectSum) { ... }
TEST_P(I32AddTest, BasicAddition_SmallNegatives_ReturnsCorrectSum) { ... }
```

**🚨 CRITICAL RULE: Avoid Redundant Test Cases**

**MANDATORY TEST EFFICIENCY PRINCIPLE**:
- **Generate TYPICAL cases only** - One representative test per category is sufficient
- **Do NOT generate exhaustive permutations** of similar test scenarios
- **Focus on DISTINCT validation concepts** rather than input variations

**Examples:**
```cpp
// ✅ CORRECT: Single representative test per concept
TEST_P(I32AddTest, BasicAddition_ReturnsCorrectSum) {
    ASSERT_EQ(call_i32_add(1, 1), 2);        // One typical positive case is sufficient
}

// ❌ FORBIDDEN: Multiple similar cases for same concept
TEST_P(I32AddTest, BasicAddition_Case1) { ASSERT_EQ(call_i32_add(1, 1), 2); }
TEST_P(I32AddTest, BasicAddition_Case2) { ASSERT_EQ(call_i32_add(1, 2), 3); }
TEST_P(I32AddTest, BasicAddition_Case3) { ASSERT_EQ(call_i32_add(1, 3), 4); }
```

**EFFICIENCY GUIDELINES:**
- **Boundary values**: Test MIN/MAX, not intermediate values
- **Sign variations**: Test positive/negative/zero, not multiple positive numbers
- **Type patterns**: Test one example per data type combination
- **Mathematical properties**: Validate the property once, not with multiple examples

### PHASE 3: Complete Code Generation

**🔒 MANDATORY PHASE 3 COMPLIANCE:**
- **NO CODE SHORTCUTS**: Generate ALL required files as specified in each step

Generate production-ready test suite through systematic implementation steps:

#### Step 3.1: Directory Structure Creation
Create the complete directory structure in `tests/unit/enhanced_opcode/{CATEGORY}/`:
```bash
# Navigate to target location
cd tests/unit/enhanced_opcode/
# Create category directory if it doesn't exist
mkdir -p {CATEGORY}
# Create subdirectories if it doesn't exist
mkdir -p {CATEGORY}/wasm-apps
```

**Required directory structure:**
```
tests/unit/enhanced_opcode/{CATEGORY}/
├── CMakeLists.txt
├── enhanced_{opcode}_test.cc
├── {opcode}_common.h (if needed)
└── wasm-apps/
    ├── {opcode}_test.wat
    └── {opcode}_test.wasm
```

**🚨 CRITICAL DIRECTORY RULE:**
- **MUST NOT create opcode subdirectories**: All opcode test files must be placed directly in `enhanced_opcode/{CATEGORY}/`
- **FORBIDDEN structure**: `tests/unit/enhanced_opcode/numeric/i32_rotr/` (creating subdirectory for individual opcode)
- **REQUIRED structure**: `tests/unit/enhanced_opcode/numeric/` (placing all numeric opcode tests directly in category directory)
- **C++ test files**: `enhanced_{opcode}_test.cc` must be at `tests/unit/enhanced_opcode/{CATEGORY}/enhanced_{opcode}_test.cc`
- **WASM/WAT files**: Must be placed in `tests/unit/enhanced_opcode/{CATEGORY}/wasm-apps/{opcode}_test.wat` and `{opcode}_test.wasm`
- **Example**:
  - `enhanced_i32_rotr_test.cc` → `tests/unit/enhanced_opcode/numeric/enhanced_i32_rotr_test.cc`
  - `i32_rotr_test.wat` → `tests/unit/enhanced_opcode/numeric/wasm-apps/i32_rotr_test.wat`
  - `i32_rotr_test.wasm` → `tests/unit/enhanced_opcode/numeric/wasm-apps/i32_rotr_test.wasm`

#### Step 3.2: GTest Framework File Generation
Generate the main C++ test file `enhanced_{opcode}_test.cc`:

**File Structure and Headers:**
- **Include proper headers**: WAMR runtime, GTest, test utilities or helper files

**Class and Method Documentation:**
- **Test fixture class**: Inherit from `testing::TestWithParam<RunningMode>` with comprehensive class documentation
- **SetUp/TearDown methods**: WAMR initialization and cleanup with RAII, including method documentation
- **Test parameter setup**: Configure interpreter and AOT modes with parameter documentation
- **Helper functions**: Common test utilities and WASM module loaders with detailed function documentation

#### Step 3.3: Common Header File Generation (if needed)
Generate `{opcode}_common.h` if shared definitions are needed:
- **Common constants**: Test values, buffer sizes, error codes
- **Shared structures**: Test data containers, helper structs
- **Utility macros**: Common assertions, test setup patterns
- **Function declarations**: Shared helper function prototypes

#### Step 3.4: WASM Test Module Generation
Generate comprehensive WASM test files:

**WAT file (`{opcode}_test.wat`):**
- **Module structure**: Complete WASM module with all required sections
- **Test functions**: Functions exercising the target opcode
- **Export declarations**: Make test functions accessible from C++
- **Memory/table setup**: Required memory or table sections (if applicable)
- **Edge case scenarios**: Include all test cases from Phase 2 planning

**WASM binary (`{opcode}_test.wasm`):**
- **Compile WAT to WASM**: Use `wat2wasm` or equivalent tool
- **Validate binary**: Ensure proper WASM format and structure
- **Test loading**: Verify WAMR can load the generated module

**🚨 CRITICAL: WASM File Path Rules**

**REQUIRED PATH PATTERN**: Use relative paths from binnary execution directory:

```cpp
// ✅ CORRECT: Simple relative path
WASM_FILE = "wasm-apps/i32_add_test.wasm";
// ❌ FORBIDDEN: Absolute paths (never use /home/user/... paths)
WASM_FILE = "/home/user/wasm-micro-runtime/tests/unit/.../i32_add_test.wasm";```
```

#### Step 3.5: CMakeLists.txt Build Configuration
Generate comprehensive build configuration:
- **WAMR build flags**: Enable required features (WAMR_BUILD_INTERP, WAMR_BUILD_AOT, etc.)
- **Feature-specific flags**: Add macros to enable the opcode being tested
- **Include unit_common.cmake**: Shared build configuration
- **Platform definitions**: Set OS-specific flags
- **Library linking**: Link against gtest_main, LLVM (if needed)
- **WASM file copying**: POST_BUILD commands to copy test files

#### Step 3.6: Test Case Implementation
Implement ALL test cases from Phase 2 with mandatory quality standards:

**🚨 MANDATORY: Test Function Documentation Requirements**  
Every test case function MUST include comprehensive documentation:

**Required Test Function Documentation Format:**
```cpp
/**
 * @test {TestCategory}_{TestScenario}_{ExpectedOutcome}
 * @brief {Brief description of what this test validates}
 * @details {Detailed explanation of test purpose and validation logic}
 * @test_category {Main/Corner/Edge/Error} - Test category from Phase 2 planning
 * @coverage_target {Specific WAMR source functions being tested}
 * @input_conditions {Description of test input setup and conditions}
 * @expected_behavior {Exact expected behavior and validation criteria}
 * @validation_method {How the test verifies correct behavior}
 */
TEST_P({Opcode}Test, {TestCategory}_{TestScenario}_{ExpectedOutcome}) {
    // Test implementation with documented steps
}
```

**Documentation Example:**
```cpp
/**
 * @test BasicAddition_ReturnsCorrectSum
 * @brief Validates i32.add produces correct arithmetic results for typical inputs
 * @details Tests fundamental addition operation with positive, negative, and mixed-sign integers.
 *          Verifies that i32.add correctly computes a + b for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_add_operation
 * @input_conditions Standard integer pairs: (5,3), (-10,-15), (20,-8)
 * @expected_behavior Returns mathematical sum: 8, -25, 12 respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32AddTest, BasicAddition_ReturnsCorrectSum) {
    // Load WASM module with i32.add test function
    wasm_module_t module = load_test_module("i32_add_test.wasm");
    ASSERT_NE(nullptr, module) << "Failed to load i32.add test module";

    // Execute test cases with documented validation
    ASSERT_EQ(8, call_i32_add(5, 3))      << "Addition of positive integers failed";
    ASSERT_EQ(-25, call_i32_add(-10, -15)) << "Addition of negative integers failed";
    ASSERT_EQ(12, call_i32_add(20, -8))    << "Addition of mixed-sign integers failed";
}
```

**Implementation Requirements:**
- **Test function documentation**: Every TEST_P function must have comprehensive header documentation
- **Assertion documentation**: Include descriptive messages for all ASSERT_* statements
- **Header file documentation**: DO NOT add comment for included header file

**CRITICAL: Mandatory ASSERT Usage Rule**  
Eliminate ALL conditional blocks in GTest cases - Use ASSERT_* exclusively:

```cpp
// ❌ FORBIDDEN: Conditional blocks in test cases
underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size,
                                   error_buf, sizeof(error_buf));
if (underflow_module) {
    // Test logic here - NEVER DO THIS
}
```

```cpp
// ✅ REQUIRED: Use ASSERT statements with descriptive messages
underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size,
                                   error_buf, sizeof(error_buf));

// For successful load scenarios:
ASSERT_NE(nullptr, underflow_module)
    << "Failed to load test module: " << error_buf;

// For expected failure scenarios:
ASSERT_EQ(nullptr, underflow_module)
    << "Expected module load to fail for invalid bytecode, but got valid module";
```

**Implementation Requirements:**
- **Main routine tests**: Implement all basic functionality test cases
- **Corner case tests**: Implement all boundary condition test cases
- **Edge case tests**: Implement all extreme scenario test cases
- **Error exception tests**: Implement all invalid scenario test cases
- **🚨 MANDATORY DOCUMENTATION**: Every TEST_P function must have comprehensive header documentation with @test, @brief, @details, @test_category, @coverage_target, @input_conditions, @expected_behavior, @validation_method tags
- **Code comments**: Add inline comments for critical operations, module loading, and function calls
- **ASSERT-only validation**: Use ASSERT_* statements exclusively (never EXPECT_*)
- **Descriptive messages**: Include context in all assertion failure messages
- **No conditional logic**: Eliminate all if/else blocks in test methods
- **Prohibited constructs**: NO GTEST_SKIP(), SUCCEED(), or FAIL() calls or any meanless code like ASSERT(true)

**CMakeLists.txt Integration Requirements:**
- **Parent CMakeLists update**: Add subdirectory to `tests/unit/enhanced_opcode/CMakeLists.txt`
- **WAMR build flags**: Enable WAMR_BUILD_INTERP, WAMR_BUILD_AOT, etc.
- **Feature-specific flags**: Add macros to enable the opcode being tested
- **Shared configuration**: Include `unit_common.cmake` for common setup
- **Platform definitions**: Set OS-specific flags if needed (e.g., `-DRUN_ON_LINUX`)
- **Library dependencies**: Link against gtest_main, LLVM (if needed)
- **WASM file deployment**: POST_BUILD commands to copy test wasm files to build directory

**Phase 3 Completion**:
Update TODO list marking tasks 3.1-3.6 as completed, display progress percentage, declare Phase 4 as next.

### PHASE 4: Build & Test Execution

**🔒 MANDATORY PHASE 4 COMPLIANCE:**
- All build and test steps MUST succeed before proceeding to Phase 6
- All source files compile without errors
- All unit tests pass (100% success rate, no failed cases)
- No runtime crashes or memory leaks detected

Execute the build and test process through systematic steps:

#### Step 4.1: Build Environment Setup
Navigate to the unit test directory and configure the build:
```bash
# Navigate to unit test root
cd tests/unit/
# Verify directory structure exists
ls -la enhanced_opcode/{CATEGORY}/
```
**Setup validation:**
- Confirm all generated files exist in correct locations
- Verify CMakeLists.txt files are properly configured
- Check WASM test files are present in wasm-apps/ directory

#### Step 4.2: CMake Configuration
Configure the build system with coverage support:
```bash
# Configure CMake with coverage enabled
cmake -S enhanced_opcode/{CATEGORY} -B build/enhanced_opcode/{CATEGORY} \
      -DCOLLECT_CODE_COVERAGE=1
```
**Configuration validation:**
- Verify CMake configuration completes without errors
- Confirm all required WAMR features are enabled
- Check coverage collection is properly configured

#### Step 4.3: Parallel Compilation
Build the test suite using parallel compilation:
```bash
# Build with parallel jobs for faster compilation
cmake --build build/enhanced_opcode/{CATEGORY} --parallel $(nproc)
```
**Compilation validation:**
- Ensure zero compilation errors
- Verify zero compilation warnings
- Confirm all test executables are generated

#### Step 4.4: Test Execution
Execute the complete test suite with detailed output:
```bash
# Run tests with verbose output and failure details
ctest --test-dir build/enhanced_opcode/{CATEGORY} \
      --output-on-failure \
      --verbose \
      --parallel $(nproc)
```
#### Step 4.5: Execution validation
**🔒 MANDATORY Execution COMPLIANCE:**
- Verify all tests pass (0 failed cases)
- Confirm no runtime crashes occur (segmentation fault, core dump, etc.)
- If any crash issues (segmentation fault, core dump, or any other runtime failures) occur during test execution, they MUST be fixed before proceeding
- Check no memory leaks are detected
- Validate test output shows expected assertions

#### Step 4.6: Results Documentation
Document build and test execution results:
- **Compilation status**: Record successful compilation
- **Test results**: Document pass/fail counts and specific test outcomes
- **Performance metrics**: Note execution times and resource usage
- **Issue identification**: Log any failures for Phase 4 resolution (if needed)

**Phase 4 Completion**: Update TODO list marking tasks 4.1-4.5 as completed. If all criteria met, proceed to Phase 5. If any failures occurred, declare Phase 4 as next.

### PHASE 5: Issue Detection & Resolution
**(Conditional - Apply only if Phase 4 fails)**

**🔒 MANDATORY PHASE 5 COMPLIANCE:**
- **ITERATIVE REQUIREMENT**: Maximum 5 resolution cycles through steps 5.1-5.4
- **CRITICAL: PRESERVE TEST INTENTION**: When fixing issues, NEVER change the test's original purpose or validation logic - only fix technical problems while maintaining the exact same test objectives and coverage goals

**SUCCESS CRITERIA (Required for Phase 6 progression):**
- ✅ All compilation errors resolved
- ✅ All runtime crashes eliminated (segmentation fault, core dump, access violations, etc.)
- ✅ All test assertions pass OR at least some tests pass (partial success acceptable)
- ✅ Build system operates correctly

**FAILURE CRITERIA (Triggers task termination):**
- ❌ Persistent build/compilation issues after 5 iterations
- ❌ All tests fail with no successful test cases
- ❌ Unresolvable runtime crashes after 5 iterations

Execute systematic issue resolution through iterative cycles:

#### Step 5.1: Comprehensive Issue Detection
Analyze all failures from Phase 4:
- **Compilation error analysis**: Parse compiler output for syntax, include, and linking errors
- **Runtime crash investigation**: Examine crash dumps, stack traces, and error messages (segmentation fault, core dump, etc.)
- **Test assertion failures**: Review failed test output and assertion messages
- **Coverage analysis issues**: Identify problems with coverage collection or reporting
- **Build system problems**: Check CMake configuration and dependency issues

#### Step 5.2: Root Cause Categorization
Classify each identified issue into specific categories:
- **Compilation issues**: Missing headers, syntax errors, linking problems
- **Runtime issues**: Null pointer dereferences, memory leaks, stack overflows
- **Crash issues**: Segmentation faults, core dumps, access violations, and any other runtime crashes that cause test binary termination
- **Test logic issues**: Incorrect expected values, wrong test setup, assertion problems
- **Build configuration issues**: Incorrect CMake settings, missing dependencies

#### Step 5.3: Targeted Resolution Application
Apply specific fixes based on issue categorization:

**🚨 CRITICAL RULE: PRESERVE TEST INTENTION AT ALL TIMES**
- **NEVER modify test objectives**: The original test purpose MUST remain unchanged
- **NEVER weaken test coverage**: Do not reduce the scope or rigor of test validation
- **NEVER change expected behavior**: Maintain the exact same validation logic and assertions
- **ONLY fix technical problems**: Address syntax, compilation, runtime issues without changing test goals

**Permitted Fix Categories:**
- **For compilation issues**: Add missing includes, fix syntax, resolve dependencies
- **For runtime issues**: Add null checks, fix memory management, validate pointers
- **🚨 For crash issues**: 
  - Fix segmentation faults, core dumps, and access violations by:
  - Adding proper null pointer checks and validation(No **if/else** block, USE ASSERT to check)
  - Fixing buffer overflows and memory access violations
  - Correcting stack overflow issues and recursive calls
  - Validating WASM module loading and execution context
  - Ensuring proper resource cleanup and memory management
- **For test logic issues**: Correct expected values ONLY if they were factually wrong, fix test setup, improve assertion messages (but not assertion logic)
- **For build issues**: Update CMakeLists.txt, add missing flags, resolve dependencies

**FORBIDDEN Fix Actions:**
- ❌ Changing test scenarios to make them "easier" to pass
- ❌ Reducing the number of test cases or assertions
- ❌ Modifying expected values to match incorrect implementation behavior
- ❌ Removing "difficult" test cases that expose real issues
- ❌ Weakening validation criteria to avoid failures
- ❌ Adding GTEST_SKIP() ro SUCCESS(), FAIL(), ASSERR(true) to bypass failing tests

#### Step 5.4: Verification & Iteration Control
Re-run build and test to confirm issue resolution:
```bash
cd tests/unit
# Clean previous build artifacts
rm -rf build/enhanced_opcode/{CATEGORY}

# Reconfigure and rebuild
cmake --log-level=ERROR -S enhanced_opcode/{CATEGORY} -B build/enhanced_opcode/{CATEGORY} -DCOLLECT_CODE_COVERAGE=1
cmake --build build/enhanced_opcode/{CATEGORY} --parallel $(nproc)
ctest --test-dir build/enhanced_opcode/{CATEGORY} --output-on-failure --verbose
```

**Verification & Decision Logic:**
1. **Verify fixes applied**: Confirm specific fixes resolve identified issues
2. **Check crash resolution**: Ensure all segmentation faults, core dumps, and runtime crashes are eliminated
3. **Validate test preservation**: Confirm test objectives and validation logic remain unchanged
4. **Assess iteration progress**: Document resolved vs. remaining issues

**Iteration Decision Tree:**
```
After verification → Evaluate results:

├─ ALL ISSUES RESOLVED?
│  └─ YES → Proceed to Phase 6 ✅
│
├─ ITERATION COUNT < 5?
│  ├─ YES → Return to Step 5.1 (next iteration) 🔄
│  └─ NO → Go to Step 5.5 (Final Resolution) ⚠️
│
└─ CRITICAL CRASHES PERSIST?
   └─ YES → Go to Step 5.6 (Task Failure) ❌
```

#### Step 5.5: Final Resolution Decision (After 5 iterations)
**When maximum iterations reached, evaluate remaining issues:**

**DECISION MATRIX:**

| Issue Status | Build Status | Test Results | Action |
|-------------|-------------|-------------|--------|
| ✅ All resolved | ✅ Success | ✅ All pass | **Proceed to Phase 6** |
| ⚠️ Test failures only | ✅ Success | ⚠️ Some pass | **Partial Success**: Comment out failing tests, proceed to Phase 6 |
| ⚠️ Test failures only | ✅ Success | ❌ All fail | **Task FAILED**: Insufficient coverage |
| ❌ Build issues | ❌ Failed | N/A | **Task FAILED**: Unresolvable build problems |
| ❌ Runtime crashes | ✅ Success | ❌ Crashes | **Task FAILED**: Unresolvable crashes |

**For Partial Success (some tests pass):**
- Identify successful vs. failing test cases
- Comment out failing test methods using `//` prefix
- Proceed to Phase 6 with working test suite

**For Task Failure:**
- Proceed to Step 5.6 for cleanup and termination

#### Step 5.6: Task Failure Cleanup (Only if task fails)
**Execute when task must be terminated due to unresolvable issues:**

**1. Document Failure Analysis:**
- Short summarize about root cause analysis of persistent failures

**2. Repository Cleanup:**
**🚨 CRITICAL**: NEVER delete committed files - only revert modifications or remove newly generated files

```bash
# Check current git status to identify file changes
git status --porcelain

# Revert modifications to committed files (DO NOT DELETE committed files)
git checkout -- {modified_files_if_any}

# Remove ONLY newly generated files (not committed files)
rm -f tests/unit/enhanced_opcode/{CATEGORY}/enhanced_{opcode}_test.cc
rm -f tests/unit/enhanced_opcode/{CATEGORY}/wasm-apps/{opcode}_test.wat
rm -f tests/unit/enhanced_opcode/{CATEGORY}/wasm-apps/{opcode}_test.wasm
# Remove newly created CMakeLists.txt only if it was newly created (not modified)
# If CMakeLists.txt existed and was modified, use git checkout instead

# Clean build artifacts
rm -rf build/enhanced_opcode/{CATEGORY}
```

**3. Task Termination:**
- Mark TODO list items as "FAILED - unresolvable issues"
- SKIP all remaining Phases (do not proceed to Phase 6)
- TERMINATE task immediately - no further work

**Phase 5 Completion**:
- **If successful**: Update TODO list marking tasks 5.1-5.5 as completed, declare Phase 6 as next
- **If failed**: Execute Step 5.6 cleanup, mark task as FAILED, terminate workflow

### PHASE 6: Code Review & Standardized Commit
Execute comprehensive final review and commit process:

#### Step 6.1: Comprehensive Code Quality Review
Perform thorough code quality assessment:
- **WAMR coding standards compliance**: Verify adherence to project conventions
- **Test description clarity**: Ensure meaningful, descriptive test names and comments
- **Resource management validation**: Confirm proper SetUp/TearDown implementation
- **Assertion quality check**: Validate meaningful ASSERT_* statements with descriptive messages
- **Code organization**: Review file structure, includes, and function organization
- **Documentation completeness**: Verify function comments and critial code comments

#### Step 6.2: Files Staging and Commit Message Creation
Prepare files for commit with standardized process:
```bash
cd tests/unit/
# Stage all generated test files(*.cc, *.wat, *.wasm CMakeLists.txt) in enhanced_opcode/{CATEGORY}/
git add enhanced_opcode/{CATEGORY}/enhanced_{OPCODE_NAME}_*.cc
git add enhanced_opcode/{CATEGORY}/wasm-apps/{OPCODE_NAME}_*.wat
git add enhanced_opcode/{CATEGORY}/wasm-apps/{OPCODE_NAME}_*.wasm
git add enhanced_opcode/{CATEGORY}/CMakeLists.txt
# Stage any header files if generated
git add {OPCODE_NAME}_common.h  # if exists

# Stage parent CMakeLists.txt if modified
git add enhanced_opcode/CMakeLists.txt  # if modified
rm -rf enhanced_opcode/{CATEGORY}/build # remove the tempory build files
```

#### Step 6.3: Commit Execution and Repository State Validation
**🔒 MANDATORY PHASE 5 COMPLIANCE**
**MUST execute commit with exact below template format, MUST NOT add any other extra content:**  
**COMMIT MESSAGE TEMPLATE (USE EXACTLY AS SHOWN)**
cmd: git commit -s -m {message}:  

```bash
"[{OPCODE_NAME}] Comprehensive test coverage

Summary
- Opcode: {OPCODE_NAME} (Category: {CATEGORY})
- Test Cases: {TEST_COUNT} comprehensive tests generated
- Files Created(or Modified): {file1}, {file2}, ...
```

**Post-commit validation:**
- **Commit verification**: Confirm commit was created successfully and 
- **Repository state check**: Verify working directory is clean
- **Commit message validation**: Ensure exact template format was used
- **File inclusion verification**: Confirm all required files are included in commit

## FINAL SUCCESS VALIDATION:
Task is complete ONLY when:
- ✅ ALL 6 phases executed in exact sequence(Conditional for Phase 5 if Phase 4 Success)
- ✅ ALL sub-steps within each phase completed fully
- ✅ TODO list properly maintained throughout entire process
- ✅ ALL quality gates and success criteria met
- ✅ Commit created using EXACT template format without adding any extra content

**This workflow demands absolute precision and complete adherence to every specified requirement.**