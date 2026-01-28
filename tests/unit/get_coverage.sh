#!/bin/bash
# Usage: ./get_coverage.sh <test_file.cc>
# Example: ./get_coverage.sh smart-tests/constants/enhanced_f32_const_test.cc

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <test_file.cc>"
    echo "Example: $0 smart-tests/constants/enhanced_f32_const_test.cc"
    exit 1
fi

TEST_FILE="$1"

# Extract the module name from the path: smart-tests/constants/xxx.cc → constants
MODULE_NAME=$(echo "$TEST_FILE" | sed -n 's|.*smart-tests/\([^/]*\)/.*|\1|p')

if [ -z "$MODULE_NAME" ]; then
    echo "Error: Unable to extract the module name from the path: $TEST_FILE"
    exit 1
fi

# Extract the class name from the test file content (the first parameter of TEST_P or TEST_F)
CLASS_NAME=$(grep -E "^(TEST_P|TEST_F)\s*\(" "$TEST_FILE" | head -1 | sed -E 's/^(TEST_P|TEST_F)\s*\(\s*([^,]+)\s*,.*/\2/')

if [ -z "$CLASS_NAME" ]; then
    echo "Error: Unable to extract the class name from the test file: $TEST_FILE"
    exit 1
fi

BUILD_DIR="build/smart-tests/$MODULE_NAME"

echo "[1/4] Cleaning up coverage data..."
find "$BUILD_DIR" -name "*.gcda" -delete 2>/dev/null || true

echo "[2/4] Running tests: $CLASS_NAME"
ctest --test-dir "$BUILD_DIR" -R "$CLASS_NAME" --output-on-failure >/dev/null 2>&1 || true

echo "[3/4] Collecting coverage..."
lcov --capture --directory "$BUILD_DIR" --output-file coverage.all.info >/dev/null 2>&1
lcov --extract coverage.all.info "*/core/iwasm/*" "*/core/shared/*" --output-file coverage.info >/dev/null 2>&1

echo "[4/4] Coverage summary:"
lcov --summary coverage.info 2>&1 | grep -E "lines|functions" | head -2

