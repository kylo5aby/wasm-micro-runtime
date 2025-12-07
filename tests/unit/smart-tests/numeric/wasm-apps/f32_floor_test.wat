(module
  ;; f32.floor comprehensive test module
  ;; Tests floor function for 32-bit floating-point values
  ;; Validates WAMR implementation across interpreter and AOT modes

  ;; Test function: Basic f32.floor operation
  ;; Input: f32 value on stack
  ;; Output: f32 floor result
  (func $test_f32_floor (param f32) (result f32)
    local.get 0    ;; Get input parameter
    f32.floor      ;; Apply floor function
  )

  ;; Export test function for C++ test access
  (export "test_f32_floor" (func $test_f32_floor))

  ;; Additional test functions for comprehensive validation

  ;; Test function: Multiple f32.floor operations in sequence
  ;; Validates stack handling and multiple operations
  (func $test_multiple_floor (param f32 f32) (result f32 f32)
    local.get 0
    f32.floor
    local.get 1
    f32.floor
  )

  ;; Export multiple operations test
  (export "test_multiple_floor" (func $test_multiple_floor))

  ;; Test function: f32.floor with constants
  ;; Tests compile-time constant folding behavior
  (func $test_floor_constants (result f32 f32 f32 f32)
    f32.const 3.7     ;; Positive fractional
    f32.floor

    f32.const -2.3    ;; Negative fractional
    f32.floor

    f32.const 5.0     ;; Integer value
    f32.floor

    f32.const 0.0     ;; Zero value
    f32.floor
  )

  ;; Export constants test
  (export "test_floor_constants" (func $test_floor_constants))

  ;; Test function: Special IEEE 754 values
  ;; Tests handling of infinity, NaN, and signed zeros
  (func $test_floor_special (param f32) (result f32)
    local.get 0
    f32.floor
  )

  ;; Export special values test
  (export "test_floor_special" (func $test_floor_special))

  ;; Test function: Precision boundary testing
  ;; Tests values around f32 precision limits
  (func $test_floor_precision (param f32) (result f32)
    local.get 0
    f32.floor
  )

  ;; Export precision test
  (export "test_floor_precision" (func $test_floor_precision))

  ;; Test function: Chain operations with f32.floor
  ;; Tests f32.floor in combination with other f32 operations
  (func $test_floor_chain (param f32) (result f32)
    local.get 0
    f32.floor      ;; Apply floor
    f32.const 1.0  ;; Add constant
    f32.add        ;; Add to floor result
    f32.floor      ;; Apply floor again
  )

  ;; Export chain operations test
  (export "test_floor_chain" (func $test_floor_chain))
)