;; WebAssembly Text Format (.wat) for i64.xor Opcode Testing
;; Comprehensive test module for 64-bit integer bitwise XOR operation
;;
;; This module provides multiple test functions to validate i64.xor behavior
;; across various scenarios including boundary values, mathematical properties, and edge cases.

(module
  ;; Primary test function for i64.xor opcode
  ;; Takes two i64 parameters and returns their XOR result
  (func (export "test_i64_xor") (param $operand1 i64) (param $operand2 i64) (result i64)
    ;; Load first operand onto stack
    local.get $operand1
    ;; Load second operand onto stack
    local.get $operand2
    ;; Execute i64.xor opcode
    ;; Stack before: [..., operand1, operand2]
    ;; Stack after:  [..., result]
    i64.xor
    ;; Return result (automatically consumed from stack)
  )

  ;; Test function for identity property (a XOR 0 = a)
  (func (export "test_identity_zero") (param $x i64) (result i64)
    local.get $x
    i64.const 0
    i64.xor
  )

  ;; Test function for self-inverse property (a XOR a = 0)
  (func (export "test_self_xor") (param $x i64) (result i64)
    local.get $x
    local.get $x
    i64.xor
  )

  ;; Test function for double XOR property (a XOR b XOR b = a)
  (func (export "test_double_xor") (param $a i64) (param $b i64) (result i64)
    local.get $a
    local.get $b
    i64.xor
    local.get $b
    i64.xor
  )

  ;; Test function for XOR with all bits set (a XOR -1 = NOT a)
  (func (export "test_xor_all_bits") (param $x i64) (result i64)
    local.get $x
    i64.const -1  ;; 0xFFFFFFFFFFFFFFFF
    i64.xor
  )

  ;; Test function for boundary values (INT64_MAX XOR INT64_MIN)
  (func (export "test_boundary_xor") (result i64)
    i64.const 0x7FFFFFFFFFFFFFFF  ;; INT64_MAX
    i64.const 0x8000000000000000  ;; INT64_MIN
    i64.xor                       ;; Should equal -1 (0xFFFFFFFFFFFFFFFF)
  )

  ;; Test function for alternating bit patterns
  (func (export "test_alternating_patterns") (result i64)
    i64.const 0x5555555555555555  ;; Alternating 01 pattern
    i64.const 0xAAAAAAAAAAAAAAAA  ;; Alternating 10 pattern
    i64.xor                       ;; Should equal 0xFFFFFFFFFFFFFFFF
  )

  ;; Test function for high 32-bit isolation
  (func (export "test_high_32bit") (result i64)
    i64.const 0xFFFFFFFF00000000  ;; High 32 bits set
    i64.const 0x0000000000000000  ;; Low 32 bits zero
    i64.xor
  )

  ;; Test function for low 32-bit isolation
  (func (export "test_low_32bit") (result i64)
    i64.const 0x00000000FFFFFFFF  ;; Low 32 bits set
    i64.const 0x0000000000000000  ;; High 32 bits zero
    i64.xor
  )

  ;; Test function for single bit toggle (power of 2 tests)
  (func (export "test_single_bit_0") (result i64)
    i64.const 0
    i64.const 1  ;; 2^0
    i64.xor
  )

  (func (export "test_single_bit_1") (result i64)
    i64.const 0
    i64.const 2  ;; 2^1
    i64.xor
  )

  (func (export "test_single_bit_63") (result i64)
    i64.const 0
    i64.const 0x8000000000000000  ;; 2^63
    i64.xor
  )

  ;; Test function for commutativity verification (a XOR b = b XOR a)
  (func (export "test_commutative_left") (param $a i64) (param $b i64) (result i64)
    local.get $a
    local.get $b
    i64.xor
  )

  (func (export "test_commutative_right") (param $a i64) (param $b i64) (result i64)
    local.get $b
    local.get $a
    i64.xor
  )

  ;; Test function for associativity verification ((a XOR b) XOR c = a XOR (b XOR c))
  (func (export "test_associative_left") (param $a i64) (param $b i64) (param $c i64) (result i64)
    local.get $a
    local.get $b
    i64.xor
    local.get $c
    i64.xor
  )

  (func (export "test_associative_right") (param $a i64) (param $b i64) (param $c i64) (result i64)
    local.get $a
    local.get $b
    local.get $c
    i64.xor
    i64.xor
  )

  ;; Test function for complex bit manipulation patterns
  (func (export "test_complex_pattern_1") (result i64)
    i64.const 0x123456789ABCDEF0
    i64.const 0x0F0F0F0F0F0F0F0F
    i64.xor
  )

  (func (export "test_complex_pattern_2") (result i64)
    i64.const 0xFEDCBA9876543210
    i64.const 0xF0F0F0F0F0F0F0F0
    i64.xor
  )
)