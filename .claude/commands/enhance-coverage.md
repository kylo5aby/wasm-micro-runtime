# WAMR Code Coverage Enhancement Command

**Invoking code-coverage-enhance subagent with validated parameters:**

**Input Configuration:**
- Module: {{module}}
- Source File: {{file}}
- Uncovered Lines: {{lines}}

**Execution Command:**
Use the **code-coverage-enhance** subagent to generate comprehensive test cases for the target code files, ensuring proper validation of all code paths and edge cases

## Output Template
Upon successful completion, only output in this format(do not output any other desciptive content):

```markdown
### Coverage Metrics For {{basename file}} - {{now format="YYYY-MM-DD-HHmm"}}
- **Module**: {{module}}
- **File Name**: {{file}}
- **Function Name**: [function_tested]
- **Lines Location**: {{lines}}
- **Baseline Coverage**: X% (Y lines covered / Z total lines)
- **Final Coverage**: A% (B lines covered / Z total lines)
- **Total Enhanced Tests**: N test cases
- **Improvement**: +N% (M additional lines covered)
- **Target Achievement**: ✅ SUCCESS / 📈 PARTIAL / ❌ FAILED
- **Files Modified**:
  - tests/unit/{{module}}/enhanced_{{basename file}}_test.cc
  - tests/unit/{{module}}/CMakeLists.txt (if applicable)

### Uncovered Code Analysis
- **Lines Still Uncovered**: [remaining line numbers]
- **Technical Limitations**: [reasons why uncovered]
- **Categorization**: Platform-specific / Critical errors / Integration-dependent
```

