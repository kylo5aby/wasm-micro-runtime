(module
  ;; Main test function for i64.popcnt operation validation
  ;; Tests population count functionality with comprehensive input coverage
  (func (export "test_i64_popcnt") (param $input i64) (result i32)
    ;; Push the input parameter onto the stack
    local.get $input
    ;; Execute i64.popcnt operation - counts set bits in 64-bit integer
    i64.popcnt
    ;; Convert i64 result to i32 (population count always fits in i32)
    i32.wrap_i64
  )

  ;; Test function for basic population count verification
  ;; Returns population count for predefined test values
  (func (export "test_basic_popcnt") (result i32)
    ;; Test value 7 (0b111) - should return 3
    i64.const 7
    i64.popcnt
    i32.wrap_i64
  )

  ;; Test function for boundary value validation
  ;; Tests extreme values and boundary conditions
  (func (export "test_boundary_popcnt") (result i32)
    ;; Test MAX_INT64 (0x7FFFFFFFFFFFFFFF) - should return 63
    i64.const 0x7FFFFFFFFFFFFFFF
    i64.popcnt
    i32.wrap_i64
  )

  ;; Test function for bit pattern analysis
  ;; Validates specific bit patterns and distributions
  (func (export "test_pattern_popcnt") (result i32)
    ;; Test alternating pattern (0xAAAAAAAAAAAAAAAA) - should return 32
    i64.const 0xAAAAAAAAAAAAAAAA
    i64.popcnt
    i32.wrap_i64
  )

  ;; Test function for mathematical property validation
  ;; Tests complement property: popcnt(a) + popcnt(~a) = 64
  (func (export "test_complement_property") (param $value i64) (result i32)
    ;; Calculate popcnt of input value
    local.get $value
    i64.popcnt

    ;; Calculate popcnt of complement (~value)
    local.get $value
    i64.const -1
    i64.xor  ;; Bitwise NOT (complement)
    i64.popcnt

    ;; Add the two results together (should equal 64)
    i64.add
    i32.wrap_i64
  )

  ;; Test function for zero operand validation
  ;; Verifies correct handling of zero input
  (func (export "test_zero_popcnt") (result i32)
    ;; Test zero - should return 0 (no bits set)
    i64.const 0
    i64.popcnt
    i32.wrap_i64
  )

  ;; Test function for all-bits-set validation
  ;; Verifies handling of maximum density bit pattern
  (func (export "test_all_bits_popcnt") (result i32)
    ;; Test all bits set (-1 or 0xFFFFFFFFFFFFFFFF) - should return 64
    i64.const -1
    i64.popcnt
    i32.wrap_i64
  )

  ;; Test function for power-of-two validation
  ;; Verifies single-bit patterns return count of 1
  (func (export "test_power_of_two_popcnt") (param $power i32) (result i32)
    ;; Create power of 2: 1 << power
    i64.const 1
    local.get $power
    i64.extend_i32_u
    i64.shl
    ;; Count bits (should be 1 for any valid power)
    i64.popcnt
    i32.wrap_i64
  )

  ;; Test function for sparse pattern validation
  ;; Tests patterns with distributed single bits
  (func (export "test_sparse_pattern_popcnt") (result i32)
    ;; Test pattern with one bit per byte (0x0101010101010101) - should return 8
    i64.const 0x0101010101010101
    i64.popcnt
    i32.wrap_i64
  )

  ;; Memory initialization for advanced testing scenarios
  (memory $mem 1)

  ;; Test function for memory-loaded value population count
  ;; Validates popcnt operation on values loaded from memory
  (func (export "test_memory_loaded_popcnt") (result i32)
    ;; Store test value in memory
    i32.const 0
    i64.const 0x123456789ABCDEF0
    i64.store

    ;; Load value from memory and count bits
    i32.const 0
    i64.load
    i64.popcnt
    i32.wrap_i64
  )
)