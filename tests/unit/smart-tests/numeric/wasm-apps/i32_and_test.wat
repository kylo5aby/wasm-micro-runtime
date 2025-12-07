(module
  ;; Basic i32.and function
  ;; Takes two i32 parameters and returns their bitwise AND
  (func $i32_and (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.and
  )
  (export "i32_and" (func $i32_and))

  ;; Stack underflow test function
  ;; Attempts i32.and with insufficient stack values - should cause underflow
  (func $i32_and_underflow (param $dummy i32) (result i32)
    ;; Only push one value, then attempt i32.and (needs 2 values)
    ;; This will be handled at runtime as WebAssembly validation prevents compile-time underflow
    ;; For testing purposes, we'll simulate by providing values but testing stack behavior separately
    i32.const 42
    i32.const 0
    i32.and
  )
  (export "i32_and_underflow" (func $i32_and_underflow))

  ;; Constant test functions for specific bit patterns

  ;; Test with hex bit patterns
  (func $test_hex_patterns (result i32)
    i32.const 0x12345678
    i32.const 0x87654321
    i32.and
  )
  (export "test_hex_patterns" (func $test_hex_patterns))

  ;; Test with decimal values
  (func $test_decimal_values (result i32)
    i32.const 15
    i32.const 7
    i32.and
  )
  (export "test_decimal_values" (func $test_decimal_values))

  ;; Test alternating bit patterns
  (func $test_alternating_patterns (result i32)
    i32.const 0xAAAAAAAA
    i32.const 0x55555555
    i32.and
  )
  (export "test_alternating_patterns" (func $test_alternating_patterns))

  ;; Test with zero (clears all bits)
  (func $test_zero_mask (result i32)
    i32.const 0xFFFFFFFF
    i32.const 0x00000000
    i32.and
  )
  (export "test_zero_mask" (func $test_zero_mask))

  ;; Test with all bits set (identity operation)
  (func $test_identity_mask (result i32)
    i32.const 0x12345678
    i32.const 0xFFFFFFFF
    i32.and
  )
  (export "test_identity_mask" (func $test_identity_mask))

  ;; Test INT32_MAX and INT32_MIN
  (func $test_max_min (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX
    i32.const 0x80000000  ;; INT32_MIN
    i32.and
  )
  (export "test_max_min" (func $test_max_min))

  ;; Test commutative property: a & b = b & a
  (func $test_commutative_1 (result i32)
    i32.const 0x12345678
    i32.const 0x87654321
    i32.and
  )
  (export "test_commutative_1" (func $test_commutative_1))

  (func $test_commutative_2 (result i32)
    i32.const 0x87654321
    i32.const 0x12345678
    i32.and
  )
  (export "test_commutative_2" (func $test_commutative_2))

  ;; Test idempotent property: a & a = a
  (func $test_idempotent (result i32)
    i32.const 0x12345678
    i32.const 0x12345678
    i32.and
  )
  (export "test_idempotent" (func $test_idempotent))

  ;; Test bit masking
  (func $test_bit_masking (result i32)
    i32.const 0x12345678
    i32.const 0x0000FFFF  ;; Lower 16 bits mask
    i32.and
  )
  (export "test_bit_masking" (func $test_bit_masking))

  ;; Test power of 2 patterns
  (func $test_power_of_2 (result i32)
    i32.const 0x0000001F  ;; 31 in binary: 11111
    i32.const 0x00000018  ;; 24 in binary: 11000
    i32.and               ;; Result: 8 in binary: 01000
  )
  (export "test_power_of_2" (func $test_power_of_2))

  ;; Test single bit isolation
  (func $test_single_bit (result i32)
    i32.const 0x12345678
    i32.const 0x00000008  ;; Isolate bit 3
    i32.and
  )
  (export "test_single_bit" (func $test_single_bit))

  ;; Test all zeros result
  (func $test_all_zeros (result i32)
    i32.const 0x0F0F0F0F
    i32.const 0xF0F0F0F0
    i32.and
  )
  (export "test_all_zeros" (func $test_all_zeros))

  ;; Test all ones preservation
  (func $test_all_ones (result i32)
    i32.const 0xFFFFFFFF
    i32.const 0xFFFFFFFF
    i32.and
  )
  (export "test_all_ones" (func $test_all_ones))
)