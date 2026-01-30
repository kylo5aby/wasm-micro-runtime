const { readFileSync, writeFileSync, appendFileSync } = require("fs");
const { execSync } = require("child_process");

const moduleName = process.argv[2];
const testFilePath = process.argv[3];
const outputFile = `/tmp/${moduleName}_redundant_check.md`;

// Get all test cases directly from ctest (no longer needs separate get_all_test_cases.js)
const ctestOutput = execSync(`ctest --test-dir build/smart-tests/${moduleName} -N`, { encoding: "utf-8" });
const testCases = ctestOutput.split("\n")
    .filter(line => line.startsWith("  Test"))
    .map(line => line.split(": ")[1])
    .filter(Boolean);

function getBaseTestName(fullName) {
    const m = fullName.match(/^(.+)\/[^/]+#GetParam\(\)=\d+$/);
    return m ? m[1] : fullName;
}

function groupByBase(testCases) {
    const byBase = {};
    for (const fullName of testCases) {
      const base = getBaseTestName(fullName);
      if (!byBase[base]) byBase[base] = [];
      byBase[base].push(fullName);
    }
    return byBase;
}

/**
 * Extract test class name from test file (e.g., "I32ConstTest" from class declaration)
 */
function getTestClassName(testFilePath) {
    const content = readFileSync(testFilePath, "utf-8");
    const m = content.match(/class\s+(\w+)\s*:[\s\S]*?testing::/);
    return m ? m[1] : null;
}

/**
 * Extract test case names directly from the test file source code.
 * Returns an array of test names (e.g., ["BasicConstantLoading_ReturnsCorrectValues", "BoundaryValues_LoadCorrectly"])
 */
function extractTestNamesFromSource(testFilePath) {
    const content = readFileSync(testFilePath, "utf-8");
    const testNames = [];
    // Match TEST_F(ClassName, TestName), TEST_P(ClassName, TestName), TEST(ClassName, TestName)
    const regex = /TEST_[FP]?\s*\(\s*\w+\s*,\s*(\w+)\s*\)/g;
    let match;
    while ((match = regex.exec(content)) !== null) {
        testNames.push(match[1]);
    }
    return testNames;
}

const byBase = groupByBase(testCases);
let baseNames = Object.keys(byBase);
const testClassName = getTestClassName(testFilePath);
if (!testClassName) {
    console.error(`Failed to get test class name from ${testFilePath}`);
    process.exit(1);
}

// Extract actual test names from source file
const sourceTestNames = extractTestNamesFromSource(testFilePath);
console.log(`Found ${sourceTestNames.length} test cases in source file: ${sourceTestNames.join(", ")}`);

// Filter baseNames to only include tests that:
// 1. Include the test class name
// 2. Have a test name that exists in the source file
baseNames = baseNames.filter(name => {
    if (!name.includes(testClassName)) return false;
    // Extract the test name part (after the last dot)
    const parts = name.split(".");
    if (parts.length < 2) return false;
    const testName = parts[parts.length - 1];
    return sourceTestNames.includes(testName);
});

let prevLineRate = 0, prevLineCoverage = 0;
const usefulTestCases = [];
const redundantTestCases = [];

for (let i = 0; i < baseNames.length; i++) {
    const testCase = baseNames[i];
    const curRunTestCases = usefulTestCases.length > 0 ? usefulTestCases.join("|") + "|" + testCase : testCase;

    execSync(`find "build/smart-tests/${moduleName}" -name "*.gcda" -delete `);
    execSync(`ctest --test-dir "build/smart-tests/${moduleName}" -R "${curRunTestCases}" --output-on-failure  >/dev/null 2>&1`);
    execSync(`lcov --capture --directory "build/smart-tests/${moduleName}" --output-file coverage.all.info  >/dev/null 2>&1`);
    execSync(`lcov --extract coverage.all.info "*/core/iwasm/*" "*/core/shared/*" --output-file coverage.info  >/dev/null 2>&1`);
    const coverage = execSync(`lcov --summary coverage.info`, { encoding: "utf-8" });

    const linesMatch = coverage.match(/lines[.\s]*:\s*([\d.]+)%\s*\((\d+)\s+of\s+(\d+)\s+lines\)/);
    const lineRate = linesMatch ? parseFloat(linesMatch[1]) : 0;   // 10.1
    const lineCovered = linesMatch ? parseInt(linesMatch[2], 10) : 0;
    if (i == 0) {
        prevLineRate = lineRate;
        prevLineCoverage = lineCovered;
        usefulTestCases.push(testCase);
        continue;
    } else if (lineCovered > prevLineCoverage) {
        usefulTestCases.push(testCase);
        prevLineRate = lineRate;
        prevLineCoverage = lineCovered;
    } else {
        redundantTestCases.push(testCase);
    }
}

writeFileSync(outputFile, `# Redundant Test Check Report\n\n`);
appendFileSync(outputFile, `## Summary\n`);

appendFileSync(outputFile, `- **Total tests:** ${baseNames.length}\n`);
appendFileSync(outputFile, `- **Useful tests:** ${usefulTestCases.length}\n`);
appendFileSync(outputFile, `- **Redundant tests:** ${redundantTestCases.length}\n`);
appendFileSync(outputFile, `\n`);
appendFileSync(outputFile, `- **Final coverage:** ${prevLineRate}% (${prevLineCoverage} lines)\n`);
appendFileSync(outputFile, `\n`);

if (redundantTestCases.length > 0) {
    appendFileSync(outputFile, `## Redundant Tests (suggest to delete)\n`);
    for (const testCase of redundantTestCases) {
        appendFileSync(outputFile, `- ❌ ${testCase}\n`);
    }
}

if (usefulTestCases.length > 0) {
    appendFileSync(outputFile, `## Useful Tests (suggest to keep)\n`);
    for (const testCase of usefulTestCases) {
        appendFileSync(outputFile, `- ✅ ${testCase}\n`);
    }
}

console.log(`========================================`);

console.log(`detection completed, check the report: ${outputFile}`);
console.log(`========================================`);