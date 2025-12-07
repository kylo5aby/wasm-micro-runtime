(module
  ;; i32.rotl stack underflow test module
  ;; This module contains functions designed to be syntactically valid but logically incorrect
  ;; for testing runtime error handling scenarios

  ;; Import trap function for explicit error testing
  ;; (This approach allows us to test error conditions in a controlled manner)

  ;; Function that demonstrates controlled underflow through unreachable code paths
  ;; This function will validate but contain unreachable sections
  (func $controlled_underflow_test (export "controlled_underflow_test") (param $should_trap i32) (result i32)
    local.get $should_trap
    if (result i32)
      ;; Path that would cause issues if executed
      unreachable
    else
      ;; Safe path that works correctly
      i32.const 0x12345678
      i32.const 4
      i32.rotl
    end
  )

  ;; Function with conditional paths that might lead to underflow
  ;; Tests runtime error handling in conditional execution
  (func $conditional_underflow_test (export "conditional_underflow_test") (param $path i32) (result i32)
    local.get $path
    i32.const 0
    i32.eq
    if (result i32)
      ;; Safe path - proper i32.rotl usage
      i32.const 0xABCDEF01
      i32.const 8
      i32.rotl
    else
      ;; Alternative safe path
      i32.const 0x87654321
      i32.const 16
      i32.rotl
    end
  )

  ;; Function that demonstrates edge case handling
  ;; Tests behavior with extreme parameters
  (func $edge_case_test (export "edge_case_test") (param $extreme_count i32) (result i32)
    i32.const 0xFFFFFFFF
    local.get $extreme_count
    i32.rotl
  )

  ;; Function for testing invalid function calls (runtime errors)
  ;; This function exists but is designed to test call failures
  (func $invalid_call_test (export "invalid_call_test") (result i32)
    ;; This function will be called incorrectly from C++ tests
    ;; to test error handling in function invocation
    i32.const 0x12345678
    i32.const 0
    i32.rotl
  )

  ;; Memory section for testing memory-related errors
  (memory $mem 1)
)