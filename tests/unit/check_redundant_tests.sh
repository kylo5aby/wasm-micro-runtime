#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

if [ $# -lt 2 ]; then
    echo "usage: $0 <MODULE_NAME> <TEST_FILE_PATH>"
    echo ""
    echo "example:"
    echo "  $0 aot-1 smart-tests/aot-1/enhanced_aot_runtime_test.cc"
    exit 1
fi

MODULE_NAME="$1"
TEST_FILE="$2"

# 生成输出文件路径
BASENAME=$(basename "$TEST_FILE" .cc)
OUTPUT_FILE="/tmp/${BASENAME}_redundant_check.md"

UNIT_TEST_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$UNIT_TEST_DIR"

if [ ! -f "$TEST_FILE" ]; then
    echo -e "${RED}error: test file not found: $TEST_FILE${NC}"
    exit 1
fi

echo "[Build] Building module: $MODULE_NAME ..."
if ! cmake --build build/smart-tests/$MODULE_NAME >/dev/null 2>&1; then
    echo -e "${RED}error: build failed for module: $MODULE_NAME${NC}"
    exit 1
fi

echo "[Step 1] get test cases from $TEST_FILE..."
mapfile -t TEST_CASES < <(grep -E "^TEST_F\(|^TEST\(|^TEST_P\(" "$TEST_FILE" | \
    sed 's/TEST_F(\([^,]*\), *\([^)]*\)).*/\1.\2/' | \
    sed 's/TEST(\([^,]*\), *\([^)]*\)).*/\1.\2/' | \
    sed 's/TEST_P(\([^,]*\), *\([^)]*\)).*/\1.\2/')

TOTAL=${#TEST_CASES[@]}
echo "found $TOTAL test cases"
echo ""

get_line_coverage() {
    lcov --summary coverage.info 2>&1 | grep "lines" | sed 's/.*: \([0-9.]*\)%.*/\1/'
}

get_line_count() {
    lcov --summary coverage.info 2>&1 | grep "lines" | sed 's/.*(\([0-9]*\) of.*/\1/'
}

declare -a USEFUL_TESTS=()  
declare -a REDUNDANT_TESTS=()
declare -a NO_COVERAGE_TESTS=()  # Tests with no coverage data or 0 lines
PREV_COVERAGE=0
PREV_LINES=0

echo "[Step 2] start incremental coverage detection..."
echo "========================================"

for ((i=0; i<TOTAL; i++)); do
    TEST_NUM=$((i + 1))
    TEST_CASE="${TEST_CASES[$i]}"
    
    echo "[${TEST_NUM}/${TOTAL}] $TEST_CASE"
    
    find "build/smart-tests/$MODULE_NAME" -name "*.gcda" -delete 2>/dev/null || true
    
    # Build ctest filter using partial matching (ctest -R supports partial match)
    # Format: TestName1|TestName2|TestName3
    # This will match all parameterized test instances (INTERP and AOT)
    TEST_FILTER=""
    for useful in "${USEFUL_TESTS[@]}"; do
        if [ -n "$TEST_FILTER" ]; then
            TEST_FILTER="${TEST_FILTER}|"
        fi
        # Use partial match to catch all parameterized instances
        TEST_FILTER="${TEST_FILTER}${useful}"
    done
    
    if [ -n "$TEST_FILTER" ]; then
        TEST_FILTER="${TEST_FILTER}|${TEST_CASE}"
    else
        TEST_FILTER="${TEST_CASE}"
    fi
    
    if ! ctest --test-dir "build/smart-tests/$MODULE_NAME" -R "$TEST_FILTER" --output-on-failure >/dev/null 2>&1; then
        echo -e "${YELLOW} warning: test execution failed, skip${NC}"
        continue
    fi
    
    lcov --capture --directory "build/smart-tests/$MODULE_NAME" --output-file coverage.all.info >/dev/null 2>&1
    lcov --extract coverage.all.info "*/core/iwasm/*" "*/core/shared/*" --output-file coverage.info >/dev/null 2>&1
    
    CURRENT_COVERAGE=$(get_line_coverage)
    CURRENT_LINES=$(get_line_count)
    
    # Check if coverage data is available
    COVERAGE_AVAILABLE=true
    if [ -z "$CURRENT_LINES" ] || ! [[ "$CURRENT_LINES" =~ ^[0-9]+$ ]]; then
        CURRENT_LINES=0
        COVERAGE_AVAILABLE=false
    fi
    
    # Ensure PREV_LINES is a valid number for comparison
    if [ -z "$PREV_LINES" ] || ! [[ "$PREV_LINES" =~ ^[0-9]+$ ]]; then
        PREV_LINES=0
    fi
    
    # Record tests with no coverage data or 0 lines
    if [ "$CURRENT_LINES" -eq 0 ] || [ "$COVERAGE_AVAILABLE" = false ]; then
        NO_COVERAGE_TESTS+=("$TEST_CASE")
    fi
        
    if [ "$TEST_NUM" -eq 1 ]; then
        USEFUL_TESTS+=("$TEST_CASE")
        PREV_COVERAGE="$CURRENT_COVERAGE"
        PREV_LINES="$CURRENT_LINES"
        continue
    fi
    
    if [ "$CURRENT_LINES" -gt "$PREV_LINES" ]; then
        NEW_LINES=$((CURRENT_LINES - PREV_LINES))
        USEFUL_TESTS+=("$TEST_CASE")
        PREV_COVERAGE="$CURRENT_COVERAGE"
        PREV_LINES="$CURRENT_LINES"
    else
        REDUNDANT_TESTS+=("$TEST_CASE")
    fi
done

USEFUL_COUNT=${#USEFUL_TESTS[@]}
REDUNDANT_COUNT=${#REDUNDANT_TESTS[@]}
NO_COVERAGE_COUNT=${#NO_COVERAGE_TESTS[@]}

{
    echo "# Redundant Test Check Report"
    echo ""
    echo "## Summary"
    echo "- **Total tests:** $TOTAL"
    echo "- **Useful tests:** $USEFUL_COUNT"
    echo "- **Redundant tests:** $REDUNDANT_COUNT"
    if [ "$NO_COVERAGE_COUNT" -gt 0 ]; then
        echo "- **Tests with no coverage data:** $NO_COVERAGE_COUNT"
    fi
    echo "- **Final coverage:** ${PREV_COVERAGE}% (${PREV_LINES} lines)"
    echo ""

    if [ "$NO_COVERAGE_COUNT" -gt 0 ]; then
        echo "## Tests with No Coverage Data (0 lines or coverage unavailable)"
        echo ""
        echo "The following test cases could not provide coverage data or have 0 line coverage:"
        echo ""
        for ((idx=0; idx<NO_COVERAGE_COUNT; idx++)); do
            echo "- ⚠️ ${NO_COVERAGE_TESTS[$idx]}"
        done
        echo ""
        echo "**Possible reasons:**"
        echo "- Test execution failed or was skipped"
        echo "- Coverage data collection failed"
        echo "- Test does not exercise any code in the target directories"
        echo "- Build configuration issues"
        echo ""
    fi

    if [ "$REDUNDANT_COUNT" -gt 0 ]; then
        echo "## Redundant Tests (suggest to delete)"
        for ((idx=0; idx<REDUNDANT_COUNT; idx++)); do
            echo "- ❌ ${REDUNDANT_TESTS[$idx]}"
        done
        echo ""
    fi

    echo "## Useful Tests"
    for ((idx=0; idx<USEFUL_COUNT; idx++)); do
        echo "- ✅ ${USEFUL_TESTS[$idx]}"
    done
} > "$OUTPUT_FILE"

echo ""
echo "========================================"
echo "detection completed"
echo "output: $OUTPUT_FILE"
echo "========================================"

if [ "$REDUNDANT_COUNT" -gt 0 ]; then
    exit 1
else
    exit 0
fi
