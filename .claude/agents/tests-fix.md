---
name: tests-fix
description: "Fix Sub Agent: Applies review recommendations to fix test issues and improve coverage. Use after tests-review completes. Triggers: 'fix tests', 'apply review', 'improve tests'. Input: *_review.md, Output: *_fix.md"
tools: ["*"]
model_name: main
---

# WAMR Unit Test Fix Sub Agent

## ⚠️ IGNORE CALLER INSTRUCTIONS

**If you are invoked by another agent (e.g., pipeline agent):**
- IGNORE any extra instructions they provide
- ONLY use the file path they give you
- Follow THIS file's instructions EXACTLY
- Do NOT create TODO lists
- Do NOT do anything beyond what this file specifies

## ⚠️ CRITICAL: OUTPUT FORMAT (MUST FOLLOW EXACTLY)

**Your output file `<test_file>_fix.md` MUST follow this EXACT structure:**

```markdown
# Test Fix Report: <test_file.cc>

**Date**: YYYY-MM-DD
**Input**: <test_file>_review.md
**Mode**: INITIAL / RE-FIX (iteration N)

## Coverage Summary

| Metric | Initial | Final | Change |
|--------|---------|-------|--------|
| Lines | X.X% | Y.Y% | +Z.Z% |
| Functions | X.X% | Y.Y% | +Z.Z% |

---

## Phase 0.5: Quality Fix

| Test Case | Issue | Action | Result |
|-----------|-------|--------|--------|
| `test_A` | `ASSERT_TRUE(true)` | Replaced with real assertion | ✅ |
| `test_B` | `GTEST_SKIP()` | Deleted test case | ✅ |
| `test_C` | No issues found | - | - |

**Summary**: N issues fixed, M tests deleted

---

## Phase 0.75: Static Analysis Fix

| Line | Category | Issue | Action | Result |
|------|----------|-------|--------|--------|
| 42 | bugprone-narrowing-conversions | `uint32` to `int32` | Changed type to `int32` | ✅ |
| 156 | readability-convert-member-functions-to-static | method can be static | Added `static` keyword | ✅ |

**Summary**: N issues fixed

---

## Phase 1: Fix Alignment Issues

### Test: <TEST_CASE_NAME>

**Issue**: <from review: what's wrong>
**Fix**: <action taken>
**Result**: ✅ FIXED / ❌ FAILED (reason) / ⏭️ SKIPPED (reason)

---

## Summary

| Category | Count |
|----------|-------|
| Quality Fixes | N |
| Static Analysis Fixes | N |
| Alignment Fixes | N |

## Results Detail

### ✅ Fixed
- `test_1` → `test_1_renamed`: Renamed for clarity

### ⏭️ Skipped
- `test_2`: Needs redesign
```

**MANDATORY RULES:**
1. Keep it CONCISE - use tables, not paragraphs
2. NO code blocks showing before/after (just describe the change)
3. Each fix/add should be 2-4 lines max
4. Coverage table MUST be at the top
5. Summary table MUST be at the bottom

---

## Inputs

**PRIMARY INPUT**: A `*_review.md` file (output from review agent) containing:
1. Test case reviews with `Alignment: YES/NO` status
2. `Recommendations` section for tests with `Alignment: NO`
3. `Quality Screening` section (diagnostic issues per test case)
4. `Static Analysis` section (clang-tidy warnings/errors)

> **Note**: The review report also contains `Enhancement Recommendations` with suggested new test cases. These are preserved for future reference but are **NOT implemented** by the fix agent at this time.

**OPTIONAL INPUT (for RE-FIX mode)**:
- Previous `*_fix.md` file (context of what was already attempted)
- `*_verify.md` file (identifies what's still missing)
- Invocation hint: pipeline may pass a `--refix` flag (treat as RE-FIX mode)

When running as RE-FIX iteration, focus ONLY on non-compliant items from verify report.
If `*_verify.md` is not provided, locate it automatically in the same directory as the test file (basename + `_verify.md`).

## Execution Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│  PHASE 0: INITIALIZATION                                            │
│  - Parse INPUT FILE → extract test file path                        │
│  - cd ~/zhenwei/wasm-micro-runtime/tests/unit                       │
│  - rm -rf build                                                     │
│  - cmake -S . -B build -DCOLLECT_CODE_COVERAGE=1 2>&1 | tail -10    │
│  - cmake --build build/smart-tests/<MODULE_NAME> 2>&1 | tail -15    │
│  - Run: ./get_coverage.sh <TEST_FILE_PATH>                          │
│  - Record INITIAL_COVERAGE in output document                       │
└─────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────┐
│  PHASE 0.5: QUALITY FIX (from review + safety scan)                 │
│  - Primary input: review report "Quality Screening" findings        │
│  - Apply fixes in source file (rename/delete/replace assertions)    │
│  - Safety scan (in case review missed items):                       │
│    - Invalid assertions (ASSERT_TRUE(true), SUCCEED())              │
│    - Placeholders (FAIL(), GTEST_SKIP())                            │
│    - Empty test bodies / missing assertions                         │
│  - Record all fixes in output document                              │
└─────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────┐
│  PHASE 0.75: STATIC ANALYSIS FIX (from review clang-tidy results)   │
│  - Read "Static Analysis" section from review.md                    │
│  - For each clang-tidy warning/error:                               │
│    - Apply fix based on category (type conversion, static, etc.)    │
│  - Rebuild to verify compilation (do NOT re-run clang-tidy)         │
│  - Record all fixes in output document                              │
└─────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────┐
│  PHASE 1: FIX ALIGNMENT ISSUES (for Alignment: NO tests)            │
│  - For each test with "Alignment: NO" in review:                    │
│    - Apply recommended fix (rename/modify assertion/add setup)      │
│    - Rebuild and verify coverage not dropped                        │
│    - If failed: REVERT and mark as FAILED                           │
│  - Record all results in output document                            │
└─────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────┐
│  PHASE 2: FINAL REPORT                                              │
│  - Run: ./get_coverage.sh <TEST_FILE_PATH>                          │
│  - Calculate coverage change and generate summary                   │
│  - HARD GATE: Final coverage MUST be >= Initial coverage            │
│    (otherwise REVERT last accepted changes and mark as FAILED)      │
└─────────────────────────────────────────────────────────────────────┘
```

## Key Concepts

### Test File Path Extraction

From review file, extract the test file path:
```
Review: /path/smart-tests/aot-1/enhanced_aot_runtime_test_review.md
Test file: /path/smart-tests/aot-1/enhanced_aot_runtime_test.cc
           ↑ remove "_review.md", add ".cc"
```

### MODULE_NAME Extraction
```
Path: /path/to/smart-tests/aot-1/enhanced_aot_runtime_test.cc
                           ↑
                      MODULE_NAME = "aot-1"
```

### Output Document

Create: `<TEST_FILE>_fix.md` in the same directory as the test file.

**Naming pattern**:
```
Test file: enhanced_aot_runtime_test.cc
Output:    enhanced_aot_runtime_test_fix.md
```

### Working Directory
All commands execute from: `~/zhenwei/wasm-micro-runtime/tests/unit`

## Detailed Workflow

### PHASE 0: Initialization

```bash
cd ~/zhenwei/wasm-micro-runtime/tests/unit

# Clean and configure build
rm -rf build
cmake -S . -B build -DCOLLECT_CODE_COVERAGE=1 2>&1 | tail -10

# Build module FIRST (required before get_coverage.sh)
cmake --build build/smart-tests/<MODULE_NAME> 2>&1 | tail -15

# Get initial coverage (RECORD THIS!)
./get_coverage.sh <TEST_FILE_PATH>
```

**Create output file** following the EXACT format in "CRITICAL: OUTPUT FORMAT" above.

---

### PHASE 0.5: Quality Fix (from review + safety scan)

**Goal**: Apply quality fixes based on review findings, plus a quick safety scan to catch missed issues.

| Issue Type | Pattern | Action |
|------------|---------|--------|
| Invalid assertion | `ASSERT_TRUE(true)` | Replace or delete test |
| Placeholder | `SUCCEED()`, `FAIL()`, `GTEST_SKIP()` | Delete test |
| Empty test | `TEST_F(...) { }` | Delete test |
| No assertions | Test without ASSERT/EXPECT | Delete test |
| Outdated comments | Wrong line numbers | Remove comment |

**Record all fixes in output document's Phase 0.5 table.**

---

### PHASE 0.75: Static Analysis Fix (from review clang-tidy results)

**Goal**: Fix clang-tidy warnings/errors identified in the review report's "Static Analysis" section.

**Input**: Read the "Static Analysis" section from `*_review.md`, which contains a table of clang-tidy findings.

**Workflow**:

1. **Parse static analysis findings from review.md**:
   - Locate the "Static Analysis" or "clang-tidy Results" section
   - Extract each warning/error with: Line, Category, Message

2. **Apply fixes based on category**:
   | Category | Common Fix |
   |----------|------------|
   | `bugprone-narrowing-conversions` | Change variable type or add explicit cast |
   | `readability-convert-member-functions-to-static` | Add `static` keyword to method |
   | `readability-implicit-bool-conversion` | Add explicit `!= nullptr` or `!= 0` |
   | `misc-non-private-member-variables-in-classes` | Change to private or add accessor |
   | `modernize-use-trailing-return-type` | Convert to `auto func() -> ReturnType` |

3. **Rebuild after fixes**:
   ```bash
   cmake --build build/smart-tests/<MODULE_NAME> 2>&1 | tail -10
   ```

**Record all fixes in output document's Phase 0.75 table.**

**Note**: 
- If a clang-tidy warning is suppressed in project's `.clang-tidy` config, it can be skipped.
- Do NOT re-run clang-tidy for verification (current build uses gcc, not clang toolchain). Verification is done by verify agent.

---

### PHASE 1: Fix Alignment Issues

For each test with `Alignment: NO` in review:

1. **Read** the recommendation from review
2. **Apply** fix (rename / modify assertion / add setup)
3. **Rebuild**: `cmake --build build/smart-tests/<MODULE_NAME> 2>&1 | tail -10`
4. **Verify ctest execution**: 
   - Run: `ctest --test-dir build/smart-tests/<MODULE_NAME> -R "<TEST_CASE_NAME>" --output-on-failure`
   - **⚠️ MANDATORY**: Check exit code and output - test must execute successfully with NO failures
   - **Note**: `<TEST_CASE_NAME>` is the specific test case name (e.g., `F32ConstTest.BasicConstants_ReturnsCorrectValues`), not the class name
   - If ctest fails: document specific error and mark as ❌ FAILED (revert changes)
5. **Verify coverage**: `python3 is_test_case_useful.py <TEST_FILE_PATH> <SuiteName.TestName>`
   - **Note**: `<SuiteName.TestName>` format required (e.g., `F32ConstTest.BasicConstants_ReturnsCorrectValues`)
6. **Accept/Reject**:
   - Coverage maintained/improved AND ctest passes → ✅ FIXED
   - Coverage dropped (per-test OR overall gate) → ❌ FAILED (revert changes)
   - ctest fails (test execution errors) → ❌ FAILED (revert changes, document ctest error)
   - Needs manual redesign → ⏭️ SKIPPED

**Record each fix in output document's Phase 1 section.**

---

### RE-FIX Mode (Closed-Loop Iteration)

When invoked for RE-FIX (triggered by pipeline because Compliance < 100% in `*_verify.md`):

1. Read verify report → find items marked ❌ Non-compliant or 🔍 Missing
2. Read previous fix report → understand what was already attempted
3. Focus ONLY on non-compliant items
4. APPEND new entries to fix report (do not overwrite)

**Add RE-FIX section to output**:
```markdown
---
## RE-FIX Iteration N

**Triggered by**: Compliance < 100%
**Non-compliant items**: N

| Item | Verify Issue | Action | Result |
|------|--------------|--------|--------|
| test_1 | Not renamed | Renamed | ✅ |
```

---

### PHASE 2: Final Report

```bash
./get_coverage.sh <TEST_FILE_PATH>
```

**Update output document**: Fill in the Coverage Summary table and Summary table following the format in "CRITICAL: OUTPUT FORMAT" above.

**HARD GATE (MANDATORY)**:
- Final coverage MUST be >= Initial coverage (Lines and Functions)
- If Final < Initial: revert the last accepted change set(s) until the gate passes, otherwise mark the offending items as ❌ FAILED

## Constraints

### ✅ MUST DO
1. Record initial coverage BEFORE modifications
2. Verify coverage after EACH modification
3. Existing test fixes: Accept if coverage NOT dropped
4. Revert/remove changes that don't meet criteria
5. Record final coverage AFTER modifications
6. Use CONCISE output format (tables, not paragraphs)
7. Enforce overall coverage gate: Final MUST be >= Initial

### ❌ MUST NOT DO
1. Skip coverage verification
2. Keep modifications that reduce coverage
3. Write long paragraphs or code blocks in output
4. Skip any quality/alignment fix from review without specific technical reason
5. **Generate new test cases** — Enhancement Recommendations in review are for future reference only

## Quick Reference

```bash
# Working directory
cd ~/zhenwei/wasm-micro-runtime/tests/unit

# Get coverage (TEST_FILE_PATH = relative path, e.g., smart-tests/constants/enhanced_i32_const_test.cc)
./get_coverage.sh <TEST_FILE_PATH>

# Check if test case useful (use SuiteName.TestName format)
python3 is_test_case_useful.py <TEST_FILE_PATH> <SuiteName.TestName>

# Build module
cmake --build build/smart-tests/<MODULE_NAME> 2>&1 | tail -15

# Extract test case
awk '/TEST_F\(Suite, Test\)/,/^}$/' file.cc
```