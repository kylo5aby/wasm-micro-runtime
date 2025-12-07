(module
  ;; Test function for basic i32.gt_s operation
  ;; Takes two i32 parameters and returns 1 if a > b (signed), 0 otherwise
  (func $test_i32_gt_s (param $a i32) (param $b i32) (result i32)
    local.get $a    ;; Push first parameter onto stack
    local.get $b    ;; Push second parameter onto stack
    i32.gt_s        ;; Perform signed greater-than comparison
  )

  ;; Test function for boundary value comparison: INT32_MAX > INT32_MIN
  ;; Returns the result of comparing maximum vs minimum 32-bit signed integers
  (func $test_boundary_comparison (result i32)
    i32.const 2147483647   ;; INT32_MAX (0x7FFFFFFF)
    i32.const -2147483648  ;; INT32_MIN (0x80000000)
    i32.gt_s              ;; Should return 1 (true)
  )

  ;; Test function for mathematical antisymmetry property validation
  ;; Verifies that if a > b, then !(b > a)
  ;; Returns 1 if antisymmetry property holds, 0 otherwise
  (func $test_antisymmetry (result i32)
    (local $result1 i32)  ;; Store result of 100 > 50
    (local $result2 i32)  ;; Store result of 50 > 100
    (local $final i32)    ;; Final validation result

    ;; Test: 100 > 50 should be true (1)
    i32.const 100
    i32.const 50
    i32.gt_s
    local.set $result1

    ;; Test: 50 > 100 should be false (0)
    i32.const 50
    i32.const 100
    i32.gt_s
    local.set $result2

    ;; Validate antisymmetry: result1 should be 1 AND result2 should be 0
    ;; If both conditions hold, return 1; otherwise return 0
    local.get $result1
    i32.const 1
    i32.eq                ;; Check if result1 == 1

    local.get $result2
    i32.const 0
    i32.eq                ;; Check if result2 == 0

    i32.and               ;; Both conditions must be true
  )

  ;; Test function for reflexivity validation: x > x should always be false
  ;; Tests multiple values to ensure reflexivity property holds
  (func $test_reflexivity (result i32)
    (local $test1 i32)    ;; Result of 0 > 0
    (local $test2 i32)    ;; Result of 42 > 42
    (local $test3 i32)    ;; Result of -100 > -100

    ;; Test: 0 > 0 should be false (0)
    i32.const 0
    i32.const 0
    i32.gt_s
    local.set $test1

    ;; Test: 42 > 42 should be false (0)
    i32.const 42
    i32.const 42
    i32.gt_s
    local.set $test2

    ;; Test: -100 > -100 should be false (0)
    i32.const -100
    i32.const -100
    i32.gt_s
    local.set $test3

    ;; All tests should return 0 for reflexivity to hold
    ;; Return 1 if all tests are 0, otherwise return 0
    local.get $test1
    local.get $test2
    i32.or                ;; test1 OR test2

    local.get $test3
    i32.or                ;; (test1 OR test2) OR test3

    i32.eqz               ;; Return 1 if all are 0 (reflexivity holds)
  )

  ;; Test function for signed vs unsigned interpretation difference
  ;; Demonstrates the importance of signed comparison for negative numbers
  (func $test_signed_interpretation (result i32)
    ;; Compare -1 and 1: in signed interpretation, -1 < 1
    ;; So -1 > 1 should return 0 (false)
    i32.const -1
    i32.const 1
    i32.gt_s
  )

  ;; Test function for zero boundary interactions
  ;; Tests various zero comparisons for edge case validation
  (func $test_zero_boundaries (param $value i32) (result i32)
    (local $greater_than_zero i32)  ;; value > 0
    (local $zero_greater i32)       ;; 0 > value

    ;; Test: value > 0
    local.get $value
    i32.const 0
    i32.gt_s
    local.set $greater_than_zero

    ;; Test: 0 > value
    i32.const 0
    local.get $value
    i32.gt_s
    local.set $zero_greater

    ;; Return combination for comprehensive testing
    ;; (value > 0) shifted left 1 bit + (0 > value)
    local.get $greater_than_zero
    i32.const 1
    i32.shl
    local.get $zero_greater
    i32.add
  )

  ;; Export all test functions for C++ test access
  (export "test_i32_gt_s" (func $test_i32_gt_s))
  (export "test_boundary_comparison" (func $test_boundary_comparison))
  (export "test_antisymmetry" (func $test_antisymmetry))
  (export "test_reflexivity" (func $test_reflexivity))
  (export "test_signed_interpretation" (func $test_signed_interpretation))
  (export "test_zero_boundaries" (func $test_zero_boundaries))
)