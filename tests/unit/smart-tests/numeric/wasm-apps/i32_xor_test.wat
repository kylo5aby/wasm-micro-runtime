(module
  ;; i32.xor comprehensive test module
  ;; Tests bitwise XOR operation with various input scenarios
  ;; Supports validation of basic operations, boundary values, and bit patterns

  ;; Main i32.xor test function - basic XOR operation
  ;; Parameters: i32 left_operand, i32 right_operand
  ;; Returns: i32 result (left_operand XOR right_operand)
  (func $i32_xor (param $left i32) (param $right i32) (result i32)
    local.get $left     ;; Push left operand to stack
    local.get $right    ;; Push right operand to stack
    i32.xor            ;; Perform XOR operation: stack[1] XOR stack[0]
  )

  ;; Test function for identity property: value XOR 0 = value
  ;; Parameters: i32 value
  ;; Returns: i32 result (should equal input value)
  (func $test_xor_identity (param $value i32) (result i32)
    local.get $value
    i32.const 0
    i32.xor
  )

  ;; Test function for self-XOR property: value XOR value = 0
  ;; Parameters: i32 value
  ;; Returns: i32 result (should equal 0)
  (func $test_xor_self (param $value i32) (result i32)
    local.get $value
    local.get $value
    i32.xor
  )

  ;; Test function for complement operation: value XOR 0xFFFFFFFF = ~value
  ;; Parameters: i32 value
  ;; Returns: i32 result (bitwise complement)
  (func $test_xor_complement (param $value i32) (result i32)
    local.get $value
    i32.const 0xFFFFFFFF
    i32.xor
  )

  ;; Test function for boundary values
  ;; Returns: i32 result of MIN_VALUE XOR MAX_VALUE
  (func $test_boundary_xor (result i32)
    i32.const 0x80000000  ;; INT32_MIN
    i32.const 0x7FFFFFFF  ;; INT32_MAX
    i32.xor
  )

  ;; Test function for alternating bit patterns
  ;; Returns: i32 result of 0x55555555 XOR 0xAAAAAAAA
  (func $test_alternating_patterns (result i32)
    i32.const 0x55555555  ;; 01010101010101...
    i32.const 0xAAAAAAAA  ;; 10101010101010...
    i32.xor
  )

  ;; Test function for double-XOR (inverse property)
  ;; Parameters: i32 original, i32 key
  ;; Returns: i32 result (should equal original after double XOR)
  (func $test_double_xor (param $original i32) (param $key i32) (result i32)
    local.get $original
    local.get $key
    i32.xor            ;; First XOR: original XOR key
    local.get $key
    i32.xor            ;; Second XOR: (original XOR key) XOR key = original
  )

  ;; Export functions for C++ test access
  (export "i32_xor" (func $i32_xor))
  (export "test_xor_identity" (func $test_xor_identity))
  (export "test_xor_self" (func $test_xor_self))
  (export "test_xor_complement" (func $test_xor_complement))
  (export "test_boundary_xor" (func $test_boundary_xor))
  (export "test_alternating_patterns" (func $test_alternating_patterns))
  (export "test_double_xor" (func $test_double_xor))
)