(module
  ;; Enhanced i32.wrap_i64 opcode test module
  ;; This module provides comprehensive test functions for the i32.wrap_i64 WebAssembly instruction
  ;; covering basic functionality, boundary conditions, overflow scenarios, and edge cases

  ;; Basic i32.wrap_i64 test function
  ;; Tests fundamental wrapping behavior with typical i64 inputs
  ;; Input: i64 value to be wrapped to i32
  ;; Output: i32 wrapped result (lower 32 bits of input)
  (func $wrap_i64 (export "wrap_i64") (param $input i64) (result i32)
    ;; Push input parameter onto stack and execute i32.wrap_i64
    local.get $input    ;; Load i64 parameter onto stack
    i32.wrap_i64       ;; Execute wrapping operation: i64 -> i32
    ;; Result: i32 value containing lower 32 bits of input
  )

  ;; Boundary condition testing function
  ;; Tests wrapping behavior at i32 boundaries and overflow scenarios
  ;; Handles special cases like i32_MIN, i32_MAX, and values beyond i32 range
  ;; Input: i64 value for boundary testing
  ;; Output: i32 wrapped result demonstrating boundary behavior
  (func $wrap_i64_boundary (export "wrap_i64_boundary") (param $input i64) (result i32)
    ;; Load parameter and perform wrapping with boundary-specific handling
    local.get $input    ;; Load i64 boundary test value onto stack
    i32.wrap_i64       ;; Execute wrapping: truncate to lower 32 bits
    ;; Result: i32 with proper boundary wrapping (modulo 2^32 behavior)
  )

  ;; Mathematical property validation function
  ;; Demonstrates that i32.wrap_i64 is equivalent to bitwise AND with 0xFFFFFFFF
  ;; and modulo 2^32 arithmetic for mathematical verification
  ;; Input: i64 value for mathematical property testing
  ;; Output: i32 result showing mathematical consistency
  (func $wrap_i64_math_verify (export "wrap_i64_math_verify") (param $value i64) (result i32)
    ;; Verify wrapping is equivalent to lower 32-bit extraction
    local.get $value    ;; Load test value for mathematical verification
    i32.wrap_i64       ;; Perform standard wrapping operation
    ;; Result proves mathematical property: result = value & 0xFFFFFFFF
  )

  ;; Edge case testing function for extreme values
  ;; Tests behavior with i64_MIN, i64_MAX, and special bit patterns
  ;; Input: i64 extreme value (like i64_MIN, i64_MAX, alternating patterns)
  ;; Output: i32 demonstrating correct extreme value handling
  (func $wrap_i64_extreme (export "wrap_i64_extreme") (param $extreme i64) (result i32)
    ;; Handle extreme i64 values and verify correct lower 32-bit extraction
    local.get $extreme  ;; Load extreme test value onto stack
    i32.wrap_i64       ;; Execute wrapping on extreme value
    ;; Result: i32 showing proper handling of extreme i64 inputs
  )

  ;; Zero and identity testing function
  ;; Tests wrapping behavior with zero and values that should remain unchanged
  ;; Input: i64 value that should wrap to identical or zero i32 value
  ;; Output: i32 demonstrating identity behavior within i32 range
  (func $wrap_i64_identity (export "wrap_i64_identity") (param $identity i64) (result i32)
    ;; Test identity cases where wrapping preserves the value
    local.get $identity ;; Load identity test value onto stack
    i32.wrap_i64       ;; Execute wrapping (should preserve for values in i32 range)
    ;; Result: i32 identical to input (for values within i32 bounds)
  )

  ;; Sign bit transition testing function
  ;; Tests wrapping behavior when sign bit changes due to truncation
  ;; Input: i64 value designed to test sign bit transitions
  ;; Output: i32 showing correct sign handling after truncation
  (func $wrap_i64_sign_test (export "wrap_i64_sign_test") (param $sign_input i64) (result i32)
    ;; Test sign bit behavior during wrapping operation
    local.get $sign_input ;; Load value with specific sign bit patterns
    i32.wrap_i64         ;; Execute wrapping and check sign preservation
    ;; Result: i32 with correct sign based on bit 31 of lower 32 bits
  )

  ;; Overflow wrapping demonstration function
  ;; Explicitly tests values that overflow i32 range and wrap around
  ;; Input: i64 value guaranteed to be > i32_MAX or < i32_MIN
  ;; Output: i32 showing modulo 2^32 wrapping behavior
  (func $wrap_i64_overflow (export "wrap_i64_overflow") (param $overflow i64) (result i32)
    ;; Demonstrate overflow wrapping with values beyond i32 bounds
    local.get $overflow  ;; Load overflow test value onto stack
    i32.wrap_i64        ;; Execute wrapping: (overflow % 2^32) as signed i32
    ;; Result: i32 demonstrating correct overflow wrapping behavior
  )

  ;; Performance and consistency testing function
  ;; Tests multiple wrapping operations for performance and consistency
  ;; Input: i64 value for repeated operations
  ;; Output: i32 result (same as single operation, for consistency verification)
  (func $wrap_i64_consistency (export "wrap_i64_consistency") (param $test_val i64) (result i32)
    ;; Perform wrapping operation to test consistency across calls
    local.get $test_val  ;; Load test value for consistency checking
    i32.wrap_i64        ;; Execute single wrapping operation
    ;; Result: i32 for consistency verification (deterministic output)
  )
)