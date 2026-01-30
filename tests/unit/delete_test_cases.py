#!/usr/bin/env python3
"""

Usage:
    python3 delete_test_cases.py <test_file.cc> <test_case1> [test_case2] [test_case3] ...

Examples:
    python3 delete_test_cases.py smart-tests/aot-1/enhanced_aot_runtime_test.cc \
        aot_resolve_import_func_SubModuleLoadFails_LogWarning \
        aot_resolve_symbols_WithAlreadyLinkedFunctions_SkipResolution
    
    # Or use SuiteName.TestName format
    python3 delete_test_cases.py smart-tests/constants/enhanced_f32_const_test.cc \
        F32ConstTest.BoundaryValues_PreservesLimits \
        F32ConstTest.SubnormalValues_PreservesAccuracy
"""

import re
import sys
from pathlib import Path


def extend_start_to_include_leading_comments(content: str, start_pos: int) -> int:
    pos = start_pos

    while True:
        i = pos
        while i > 0 and content[i - 1].isspace():
            i -= 1
        gap = content[i:pos]
        if gap.count("\n") > 2:
            break

        if i >= 2 and content[i - 2:i] == "*/":
            comment_start = content.rfind("/*", 0, i - 2)
            if comment_start != -1:
                line_start = content.rfind("\n", 0, comment_start) + 1
                if content[line_start:comment_start].strip() == "":
                    comment_start = line_start

                pos = comment_start
                continue

        line_start = content.rfind("\n", 0, i) + 1
        line_text = content[line_start:i]
        if line_text.lstrip().startswith("//"):
            pos = line_start
            continue

        break

    return pos


def find_test_case_range(content: str, suite_name: str, test_name: str) -> tuple[int, int] | None:
    """Find the start and end positions of a test case, return (start_pos, end_pos) or None"""
    # Match TEST_F, TEST, TEST_P
    for prefix in ['TEST_F', 'TEST', 'TEST_P']:
        pattern = rf'{prefix}\s*\(\s*{re.escape(suite_name)}\s*,\s*{re.escape(test_name)}\s*\)'
        match = re.search(pattern, content)
        if match:
            break
    else:
        return None
    
    start_pos = match.start()
    
    # Find the start of the function body {
    brace_start = content.find('{', match.end())
    if brace_start == -1:
        return None
    
    # Match the closing brace to find the end of the function body
    brace_count = 0
    pos = brace_start
    in_string = False
    in_char = False
    in_line_comment = False
    in_block_comment = False
    prev_char = ''
    
    while pos < len(content):
        char = content[pos]
        
        if prev_char == '\\':
            prev_char = ''
            pos += 1
            continue
        
        if in_line_comment:
            if char == '\n':
                in_line_comment = False
            pos += 1
            prev_char = char
            continue
        
        if in_block_comment:
            if char == '/' and prev_char == '*':
                in_block_comment = False
            pos += 1
            prev_char = char
            continue
        
        if char == '/' and pos + 1 < len(content):
            next_char = content[pos + 1]
            if next_char == '/' and not in_string and not in_char:
                in_line_comment = True
                pos += 2
                prev_char = next_char
                continue
            elif next_char == '*' and not in_string and not in_char:
                in_block_comment = True
                pos += 2
                prev_char = next_char
                continue
        
        if char == '"' and not in_char:
            in_string = not in_string
        if char == "'" and not in_string:
            in_char = not in_char
        
        if not in_string and not in_char:
            if char == '{':
                brace_count += 1
            elif char == '}':
                brace_count -= 1
                if brace_count == 0:
                    end_pos = pos + 1
                    # Skip the following empty lines
                    while end_pos < len(content) and content[end_pos] in ' \t':
                        end_pos += 1
                    if end_pos < len(content) and content[end_pos] == '\n':
                        end_pos += 1
                    return (start_pos, end_pos)
        
        prev_char = char
        pos += 1
    
    return None


def parse_test_case_name(test_case: str, content: str) -> tuple[str, str] | None:
    """
    Parse the test case name, return (suite_name, test_name)
    
    Supports multiple formats:
    1. SuiteName.TestName (e.g. F32ConstTest.BoundaryValues_PreservesLimits)
    2. Full test case name (e.g. EnhancedAotRuntimeTest.aot_resolve_import_func_SubModuleLoadFails_LogWarning)
    3. Only test name (e.g. aot_resolve_import_func_SubModuleLoadFails_LogWarning)
       In this case, the corresponding TEST_F/TEST_P definition needs to be found in the test file
    """
    # If it contains a dot, parse it as SuiteName.TestName
    if '.' in test_case:
        parts = test_case.split('.', 1)
        return (parts[0], parts[1])
    
    # Otherwise, find the corresponding TEST_F/TEST_P definition in the test file
    # Search all TEST_F/TEST_P definitions, find the matching test case name
    patterns = [
        r'TEST_F\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)',
        r'TEST_P\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)',
        r'TEST\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)',
    ]
    
    # First try exact matching
    for pattern in patterns:
        for match in re.finditer(pattern, content):
            suite_name = match.group(1)
            test_name = match.group(2)
            full_name = f"{suite_name}.{test_name}"
            # Exact matching
            if test_case == full_name or test_case == test_name:
                return (suite_name, test_name)
    
    # If exact matching fails, try partial matching (test_case is a substring of test_name or contains the key part of test_name)
    # Prioritize matching the longest common suffix
    best_match = None
    best_match_len = 0
    
    for pattern in patterns:
        for match in re.finditer(pattern, content):
            suite_name = match.group(1)
            test_name = match.group(2)
            # Check if test_case is a suffix of test_name, or test_name contains the main part of test_case
            if test_case in test_name:
                # Calculate the match length (from the end)
                match_len = len(test_case)
                if match_len > best_match_len:
                    best_match = (suite_name, test_name)
                    best_match_len = match_len
            elif test_name.endswith(test_case.split('_')[-1] if '_' in test_case else test_case):
                # Match the part after the last underscore
                match_len = len(test_case.split('_')[-1])
                if match_len > best_match_len:
                    best_match = (suite_name, test_name)
                    best_match_len = match_len
    
    return best_match


def delete_test_cases(test_file: str, test_cases: list[str]) -> None:
    """Delete the specified test cases from the test file"""
    if not test_cases:
        print("No test cases to delete")
        return
    
    print(f"Preparing to delete {len(test_cases)} test cases")
    
    # Read the test file
    with open(test_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    deleted = []
    not_found = []
    
    for test_case in test_cases:
        # Parse the test case name
        parsed = parse_test_case_name(test_case, content)
        if parsed is None:
            not_found.append(test_case)
            continue
        
        suite_name, test_name = parsed
        
        # Find and delete
        range_result = find_test_case_range(content, suite_name, test_name)
        if range_result is None:
            not_found.append(test_case)
            continue
        
        start_pos, end_pos = range_result
        start_pos = extend_start_to_include_leading_comments(content, start_pos)
        content = content[:start_pos] + content[end_pos:]
        deleted.append(test_case)
    
    # Clean up extra empty lines
    content = re.sub(r'\n{3,}', '\n\n', content)
    
    # Write back to the file
    with open(test_file, 'w', encoding='utf-8') as f:
        f.write(content)
    
    # Output the result
    print(f"\n✅ Successfully deleted {len(deleted)} test cases:")
    for test in deleted:
        print(f"   - {test}")
    
    if not_found:
        print(f"\n⚠️ Not found {len(not_found)} test cases:")
        for test in not_found:
            print(f"   - {test}")


def main():
    if len(sys.argv) < 3:
        print("Usage: python3 delete_test_cases.py <test_file.cc> <test_case1> [test_case2] ...")
        print("Examples: python3 delete_test_cases.py smart-tests/aot-1/test.cc test_case1 test_case2")
        print("      python3 delete_test_cases.py smart-tests/constants/test.cc SuiteName.TestName1 SuiteName.TestName2")
        sys.exit(1)
    
    test_file = sys.argv[1]
    test_cases = sys.argv[2:]
    
    if not Path(test_file).exists():
        print(f"Error: test file not found: {test_file}")
        sys.exit(1)
    
    delete_test_cases(test_file, test_cases)


if __name__ == '__main__':
    main()
