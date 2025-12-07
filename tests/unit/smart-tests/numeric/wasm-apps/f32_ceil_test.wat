(module
  ;; f32.ceil comprehensive test module
  ;; Tests ceiling function for 32-bit floating-point values
  ;; Validates WAMR implementation across interpreter and AOT modes

  ;; Test function: Basic f32.ceil operation
  ;; Input: f32 value on stack
  ;; Output: f32 ceiling result
  (func $test_f32_ceil (param f32) (result f32)
    local.get 0    ;; Get input parameter
    f32.ceil       ;; Apply ceiling function
  )

  ;; Export test function for C++ test access
  (export "test_f32_ceil" (func $test_f32_ceil))

  ;; Additional test functions for comprehensive validation

  ;; Test function: Multiple f32.ceil operations in sequence
  ;; Validates stack handling and multiple operations
  (func $test_multiple_ceil (param f32 f32) (result f32 f32)
    local.get 0
    f32.ceil
    local.get 1
    f32.ceil
  )

  ;; Export multiple operations test
  (export "test_multiple_ceil" (func $test_multiple_ceil))

  ;; Test function: f32.ceil with constants
  ;; Tests compile-time constant folding behavior
  (func $test_ceil_constants (result f32 f32 f32 f32)
    f32.const 1.3     ;; Positive fractional
    f32.ceil

    f32.const -2.7    ;; Negative fractional
    f32.ceil

    f32.const 5.0     ;; Integer value
    f32.ceil

    f32.const 0.0     ;; Zero value
    f32.ceil
  )

  ;; Export constants test
  (export "test_ceil_constants" (func $test_ceil_constants))

  ;; Test function: Special IEEE 754 values
  ;; Tests handling of infinity, NaN, and signed zeros
  (func $test_ceil_special (param f32) (result f32)
    local.get 0
    f32.ceil
  )

  ;; Export special values test
  (export "test_ceil_special" (func $test_ceil_special))

  ;; Test function: Precision boundary testing
  ;; Tests values around f32 precision limits
  (func $test_ceil_precision (param f32) (result f32)
    local.get 0
    f32.ceil
  )

  ;; Export precision test
  (export "test_ceil_precision" (func $test_ceil_precision))

  ;; Test function: Chain operations with f32.ceil
  ;; Tests f32.ceil in combination with other f32 operations
  (func $test_ceil_chain (param f32) (result f32)
    local.get 0
    f32.ceil       ;; Apply ceiling
    f32.const 1.0  ;; Add constant
    f32.add        ;; Add to ceiling result
    f32.ceil       ;; Apply ceiling again
  )

  ;; Export chain operations test
  (export "test_ceil_chain" (func $test_ceil_chain))
)