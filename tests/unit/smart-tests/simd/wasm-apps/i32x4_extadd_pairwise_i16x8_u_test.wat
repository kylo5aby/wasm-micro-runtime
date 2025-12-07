(module
  ;; Import memory for SIMD operations
  (memory (export "memory") 1)

  ;; Test function for basic i32x4.extadd_pairwise_i16x8_u opcode
  ;; Parameters: one v128 value representing i16x8 vector
  ;; Returns: v128 value representing i32x4 result with pairwise sums
  (func $basic_pairwise (export "basic_pairwise")
    (param $input v128) (result v128)
    local.get $input
    i32x4.extadd_pairwise_i16x8_u
  )

  ;; Test function for boundary values validation
  ;; Parameters: one v128 value representing i16x8 vector
  ;; Returns: v128 value representing i32x4 result
  (func $boundary_values (export "boundary_values")
    (param $input v128) (result v128)
    local.get $input
    i32x4.extadd_pairwise_i16x8_u
  )

  ;; Test function for zero operands validation
  ;; Parameters: one v128 value representing i16x8 vector
  ;; Returns: v128 value representing i32x4 result
  (func $zero_operands (export "zero_operands")
    (param $input v128) (result v128)
    local.get $input
    i32x4.extadd_pairwise_i16x8_u
  )

  ;; Test function for maximum values and extension validation
  ;; Parameters: one v128 value representing i16x8 vector
  ;; Returns: v128 value representing i32x4 result
  (func $maximum_values (export "maximum_values")
    (param $input v128) (result v128)
    local.get $input
    i32x4.extadd_pairwise_i16x8_u
  )

  ;; Test function for constant values to validate known results
  (func $test_constant_sequential (export "test_constant_sequential") (result v128)
    ;; Create i16x8 vector [1,2,3,4,5,6,7,8]
    ;; Expected result: [3,7,11,15] as i32x4
    v128.const i16x8 1 2 3 4 5 6 7 8
    i32x4.extadd_pairwise_i16x8_u
  )

  ;; Test function for maximum boundary case
  (func $test_constant_max (export "test_constant_max") (result v128)
    ;; Create i16x8 vector with all maximum values [65535,65535,65535,65535,65535,65535,65535,65535]
    ;; Expected result: [131070,131070,131070,131070] as i32x4
    v128.const i16x8 65535 65535 65535 65535 65535 65535 65535 65535
    i32x4.extadd_pairwise_i16x8_u
  )

  ;; Test function for zero case
  (func $test_constant_zero (export "test_constant_zero") (result v128)
    ;; Create i16x8 vector with all zeros [0,0,0,0,0,0,0,0]
    ;; Expected result: [0,0,0,0] as i32x4
    v128.const i16x8 0 0 0 0 0 0 0 0
    i32x4.extadd_pairwise_i16x8_u
  )

  ;; Test function for mixed min/max pattern
  (func $test_constant_mixed (export "test_constant_mixed") (result v128)
    ;; Create i16x8 vector [0,65535,65535,0,0,65535,65535,0]
    ;; Expected result: [65535,65535,65535,65535] as i32x4
    v128.const i16x8 0 65535 65535 0 0 65535 65535 0
    i32x4.extadd_pairwise_i16x8_u
  )

  ;; Test function for power of 2 patterns
  (func $test_constant_powers (export "test_constant_powers") (result v128)
    ;; Create i16x8 vector [32768,32768,16384,16384,8192,8192,4096,4096]
    ;; Expected result: [65536,32768,16384,8192] as i32x4
    v128.const i16x8 32768 32768 16384 16384 8192 8192 4096 4096
    i32x4.extadd_pairwise_i16x8_u
  )

  ;; Test function with stack manipulation for thorough validation
  (func $test_stack_operations (export "test_stack_operations")
    (param $input1 v128) (param $input2 v128) (result v128)
    ;; Test pairwise add on first input, then add with second input
    local.get $input1
    i32x4.extadd_pairwise_i16x8_u
    ;; Convert second input to i32x4 and add
    local.get $input2
    i32x4.extadd_pairwise_i16x8_u
    i32x4.add
  )

  ;; Test function that exercises the opcode in a computation chain
  (func $test_computation_chain (export "test_computation_chain")
    (param $input v128) (result v128)
    ;; Apply pairwise addition twice to demonstrate correctness
    local.get $input
    i32x4.extadd_pairwise_i16x8_u
    ;; Convert result back to i16x8 interpretation and apply again
    ;; (This tests that the result format is correct)
    i16x8.extend_low_i8x16_u
    i32x4.extadd_pairwise_i16x8_u
  )
)