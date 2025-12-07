(module
  ;; Test module for i64.extend_i32_u opcode validation
  ;; Provides comprehensive test functions for zero-extension from i32 to i64

  ;; Import memory for test operations (optional, not used in this test)
  (memory (export "memory") 1)

  ;; Main test function: i64.extend_i32_u with single i32 input
  ;; @param $input (i32) Input value to be zero-extended to i64
  ;; @return (i64) Zero-extended i64 result
  (func $test_extend_i32_u (export "test_extend_i32_u") (param $input i32) (result i64)
    ;; Push input parameter onto stack
    local.get $input

    ;; Execute i64.extend_i32_u opcode - zero extend i32 to i64
    ;; This is the core operation being tested:
    ;; - Takes i32 from stack top
    ;; - Zero-extends to i64 (upper 32 bits become 0)
    ;; - Pushes i64 result back to stack
    i64.extend_i32_u

    ;; Return i64 result
  )

  ;; Test function for stack underflow validation
  ;; This function will be used to create invalid bytecode at runtime
  ;; for testing stack underflow conditions
  (func $test_stack_underflow (export "test_stack_underflow") (result i32)
    ;; Return a constant - actual stack underflow testing will be done
    ;; by loading malformed bytecode modules in the C++ test
    i32.const 0
  )

  ;; Additional test functions for specific boundary cases

  ;; Test zero extension of literal zero
  (func $test_zero_extension (export "test_zero_extension") (result i64)
    i32.const 0
    i64.extend_i32_u
  )

  ;; Test zero extension of UINT32_MAX (0xFFFFFFFF)
  (func $test_max_u32_extension (export "test_max_u32_extension") (result i64)
    i32.const -1  ;; 0xFFFFFFFF in i32
    i64.extend_i32_u
  )

  ;; Test zero extension of INT32_MIN bit pattern (0x80000000)
  ;; Should result in 0x0000000080000000 (positive i64), not sign-extended
  (func $test_int32_min_pattern (export "test_int32_min_pattern") (result i64)
    i32.const -2147483648  ;; 0x80000000 bit pattern
    i64.extend_i32_u
  )

  ;; Test zero extension of INT32_MAX (0x7FFFFFFF)
  (func $test_int32_max_extension (export "test_int32_max_extension") (result i64)
    i32.const 2147483647  ;; 0x7FFFFFFF
    i64.extend_i32_u
  )

  ;; Test function with multiple extend operations in sequence
  ;; Validates that multiple operations work correctly
  (func $test_multiple_extensions (export "test_multiple_extensions")
        (param $input1 i32) (param $input2 i32) (result i64 i64)
    ;; First extension
    local.get $input1
    i64.extend_i32_u

    ;; Second extension
    local.get $input2
    i64.extend_i32_u
  )

  ;; Test function combining extend with other i64 operations
  ;; Validates that extended values work correctly with other opcodes
  (func $test_extend_and_add (export "test_extend_and_add")
        (param $a i32) (param $b i32) (result i64)
    ;; Extend first operand
    local.get $a
    i64.extend_i32_u

    ;; Extend second operand
    local.get $b
    i64.extend_i32_u

    ;; Add the extended values
    i64.add
  )

  ;; Test function for bit pattern validation
  ;; Tests specific bit patterns to ensure proper zero extension
  (func $test_bit_patterns (export "test_bit_patterns") (result i64 i64 i64)
    ;; Test alternating bit pattern 0xAAAAAAAA
    i32.const -1431655766  ;; 0xAAAAAAAA
    i64.extend_i32_u

    ;; Test alternating bit pattern 0x55555555
    i32.const 1431655765   ;; 0x55555555
    i64.extend_i32_u

    ;; Test mixed bit pattern 0xF0F0F0F0
    i32.const -252645136   ;; 0xF0F0F0F0
    i64.extend_i32_u
  )
)