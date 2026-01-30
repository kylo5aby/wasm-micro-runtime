#!/usr/bin/env python3
"""
检查某个测试用例是否对覆盖率有贡献（相对于同一测试文件的其他测试）

Usage:
    python3 is_test_case_useful.py <test_file.cc> <test_case_name>

Example:
    python3 is_test_case_useful.py smart-tests/aot-1/enhanced_aot_runtime_test.cc \
        EnhancedAotRuntimeTest.aot_resolve_import_func_Success

Output (useful):
    ✅ USEFUL: Test case contributes to coverage
    New lines covered: 15
    Coverage change: +0.05%

Output (not useful):
    ❌ NOT USEFUL: Test case does not contribute to coverage
    New lines covered: 0
    Coverage change: 0%
"""

import subprocess
import sys
import re
from pathlib import Path


def get_module_name(test_file: str) -> str:
    """Extract the module name from the test file path"""
    parts = Path(test_file).parts
    for i, part in enumerate(parts):
        if part == 'smart-tests' and i + 1 < len(parts):
            return parts[i + 1]
    return None


def get_suite_name_from_test_case(test_case: str) -> str:
    """Extract the suite name from the test case name"""
    # EnhancedAotRuntimeTest.aot_memory_init_xxx -> EnhancedAotRuntimeTest
    if '.' in test_case:
        return test_case.split('.')[0]
    return test_case


def run_command(cmd: str) -> tuple[int, str]:
    """Run the command and return (return code, output)"""
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.returncode, result.stdout + result.stderr


def get_covered_lines(coverage_file: str) -> set[tuple[str, int]]:
    """Extract the covered lines (file, line number) from the coverage.info file"""
    covered = set()
    current_file = None
    
    try:
        with open(coverage_file, 'r') as f:
            for line in f:
                line = line.strip()
                if line.startswith('SF:'):
                    current_file = line[3:]
                elif line.startswith('DA:') and current_file:
                    parts = line[3:].split(',')
                    if len(parts) >= 2 and int(parts[1]) > 0:
                        covered.add((current_file, int(parts[0])))
    except FileNotFoundError:
        pass
    
    return covered


def get_covered_functions(coverage_file: str) -> set[tuple[str, str]]:
    """Extract the covered functions (file, function name) from the coverage.info file"""
    covered = set()
    current_file = None
    
    try:
        with open(coverage_file, 'r') as f:
            for line in f:
                line = line.strip()
                if line.startswith('SF:'):
                    current_file = line[3:]
                elif line.startswith('FNDA:') and current_file:
                    # FNDA:count,function_name
                    parts = line[5:].split(',', 1)
                    if len(parts) >= 2 and int(parts[0]) > 0:
                        covered.add((current_file, parts[1]))
    except FileNotFoundError:
        pass
    
    return covered


def run_tests_except_one(module_name: str, suite_name: str, exclude_test: str) -> tuple[set[tuple[str, int]], set[tuple[str, str]]]:
    """Run all tests in the test file except the specified test, return the covered (line, function)"""
    build_dir = f"build/smart-tests/{module_name}"
    
    # Clean up the coverage data
    run_command(f"find {build_dir} -name '*.gcda' -delete 2>/dev/null")
    
    # Run only the tests in the suite, but exclude the specified test
    # ctest -R "SuiteName" -E "^SuiteName\.TestName$"
    # Note: only escape the dot, don't use re.escape (it will produce double backslashes)
    exclude_pattern = exclude_test.replace('.', r'\.')
    run_command(f"ctest --test-dir {build_dir} -R '{suite_name}' -E '^{exclude_pattern}$' --output-on-failure 2>&1")
    
    # Collect the coverage
    run_command(f"lcov --capture --directory {build_dir} --output-file coverage.without.info 2>&1")
    run_command(f"lcov --extract coverage.without.info '*/core/iwasm/*' '*/core/shared/*' --output-file coverage.without.filtered.info 2>&1")
    
    return get_covered_lines("coverage.without.filtered.info"), get_covered_functions("coverage.without.filtered.info")


def run_all_tests_in_file(module_name: str, suite_name: str) -> tuple[set[tuple[str, int]], set[tuple[str, str]]]:
    """Run all tests in the test file, return the covered (line, function)"""
    build_dir = f"build/smart-tests/{module_name}"
    
    # Clean up the coverage data
    run_command(f"find {build_dir} -name '*.gcda' -delete 2>/dev/null")
    
    # Run only the tests in the suite
    run_command(f"ctest --test-dir {build_dir} -R '{suite_name}' --output-on-failure 2>&1")
    
    # Collect the coverage
    run_command(f"lcov --capture --directory {build_dir} --output-file coverage.all.info 2>&1")
    run_command(f"lcov --extract coverage.all.info '*/core/iwasm/*' '*/core/shared/*' --output-file coverage.all.filtered.info 2>&1")
    
    return get_covered_lines("coverage.all.filtered.info"), get_covered_functions("coverage.all.filtered.info")


def check_test_usefulness(test_file: str, test_case: str) -> dict:
    module_name = get_module_name(test_file)
    if not module_name:
        print(f"Error: Unable to extract the module name from the path: {test_file}", file=sys.stderr)
        sys.exit(1)
    
    suite_name = get_suite_name_from_test_case(test_case)
    
    # Get the covered (line, function) of all tests in the test file
    all_lines, all_funcs = run_all_tests_in_file(module_name, suite_name)
    
    # Get the covered (line, function) after excluding the specified test
    without_lines, without_funcs = run_tests_except_one(module_name, suite_name, test_case)
    
    # Calculate the unique lines and functions contributed by the test
    unique_lines = all_lines - without_lines
    unique_funcs = all_funcs - without_funcs
    
    # Test is USEFUL if it contributes to either lines OR functions
    result = {
        'useful': len(unique_lines) > 0 or len(unique_funcs) > 0,
        'new_lines': len(unique_lines),
        'new_funcs': len(unique_funcs),
        'total_lines': len(all_lines),
        'total_funcs': len(all_funcs),
        'without_lines': len(without_lines),
        'without_funcs': len(without_funcs),
    }
    
    if result['total_lines'] > 0:
        result['line_change_percent'] = (len(unique_lines) / result['total_lines']) * 100
    else:
        result['line_change_percent'] = 0.0
    
    if result['total_funcs'] > 0:
        result['func_change_percent'] = (len(unique_funcs) / result['total_funcs']) * 100
    else:
        result['func_change_percent'] = 0.0
    
    return result


def main():
    if len(sys.argv) != 3:
        print("Usage: python3 is_test_case_useful.py <test_file.cc> <test_case_name>")
        print("Example: python3 is_test_case_useful.py smart-tests/aot-1/test.cc SuiteName.TestName")
        sys.exit(1)
    
    test_file = sys.argv[1]
    test_case = sys.argv[2]
    
    if not Path(test_file).exists():
        print(f"Error: File not found: {test_file}", file=sys.stderr)
        sys.exit(1)
    
    result = check_test_usefulness(test_file, test_case)
    
    if result['useful']:
        print(f"✅ USEFUL: Test case contributes to coverage")
        print(f"New lines covered: {result['new_lines']} (+{result['line_change_percent']:.2f}%)")
        print(f"New functions covered: {result['new_funcs']} (+{result['func_change_percent']:.2f}%)")
        sys.exit(0)
    else:
        print(f"❌ NOT USEFUL: Test case does not contribute to coverage")
        print(f"New lines covered: 0")
        print(f"New functions covered: 0")
        sys.exit(1)


if __name__ == '__main__':
    main()
