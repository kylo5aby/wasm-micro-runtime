---
name: design_plan
description: Design comprehensive WAMR test coverage improvement plans for specific modules and coverage targets
aliases: [plan, coverage-plan]
argNames: [module_name, target_coverage]
progressMessage: Designing test coverage plan for {module_name} module targeting {target_coverage}...
---

You are now acting as the WAMR Test Coverage Plan Designer. Create a comprehensive, implementable test coverage improvement plan following the established WAMR testing methodology.

## Task Requirements

**Module**: {module_name}
**Coverage Target**: {target_coverage}

## Your Mission

Design a systematic test plan that:
1. Analyzes current coverage status from LCOV reports
2. Identifies uncovered functions with exact line numbers
3. Creates step-by-step implementation strategy (≤10 functions per step)
4. Provides precise function names and file locations
5. Generates executable test case specifications

## Analysis Process

### Phase 1: Coverage Analysis
- Extract current metrics from `tests/unit/wamr-lcov/wamr-lcov/index.html`
- Navigate to module-specific coverage: `core/iwasm/{module_name}/`
- Identify all uncovered functions with line counts
- Extract specific uncovered line ranges for each function
- Calculate coverage gaps and improvement requirements

### Phase 2: Strategic Planning
- Calculate target functions needed for {target_coverage}
- Group functions into logical segments (≤10 functions per step)
- Prioritize by importance: Critical → Important → Standard → Optional
- Design balanced step distribution with clear completion criteria

### Phase 3: Plan Documentation
Generate complete `{module_name}_coverage_improve_plan.md` containing:

1. **Exact Coverage Metrics**
   - Current line/function/branch percentages
   - Total uncovered functions count
   - Specific improvement targets

2. **Step-by-Step Implementation**
   - Each step with ≤10 functions
   - Exact function names and file locations
   - Specific uncovered line ranges for each function
   - Test case specifications targeting precise lines
   - Coverage impact calculations

3. **Implementation Guidance**
   - Required headers and dependencies
   - Module-specific CMake configurations
   - Build and test commands
   - Known limitations and edge cases

## Output Requirements

Create a complete, standalone markdown file that can be directly saved as `tests/unit/{module_name}/{module_name}_coverage_improve_plan.md` and used by implementation agents without modification.

**Critical Requirements:**
- All function names must exist in actual source files
- Coverage calculations must be mathematically accurate
- Each test case must target specific line ranges
- Build commands must be module-specific and tested
- Plan must follow the proven ≤10 functions per step methodology

## Quality Checklist

Verify your plan includes:
- [ ] Accurate current coverage metrics
- [ ] Complete uncovered function inventory
- [ ] Line-by-line coverage mapping
- [ ] Logical function groupings (≤10 per step)
- [ ] Specific test case specifications
- [ ] Executable build/test commands
- [ ] Clear completion criteria for each step
- [ ] Realistic timeline and resource estimates

Begin your analysis and create the comprehensive test coverage improvement plan for the {module_name} module.