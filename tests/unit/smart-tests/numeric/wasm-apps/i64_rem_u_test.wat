(module
  ;; Function to test i64.rem_u opcode
  ;; Takes two i64 parameters (dividend, divisor) and returns i64 remainder
  (func (export "i64_rem_u_test") (param $dividend i64) (param $divisor i64) (result i64)
    ;; Push dividend and divisor onto stack
    local.get $dividend
    local.get $divisor

    ;; Perform i64.rem_u operation
    ;; Computes unsigned remainder: dividend % divisor
    ;; Both operands treated as unsigned 64-bit values
    i64.rem_u
  )

  ;; Additional test functions for comprehensive validation

  ;; Test function for basic unsigned remainder
  (func (export "test_basic_remainder") (result i64)
    i64.const 10
    i64.const 3
    i64.rem_u
  )

  ;; Test function for larger numbers
  (func (export "test_large_remainder") (result i64)
    i64.const 100
    i64.const 7
    i64.rem_u
  )

  ;; Test function for very large numbers
  (func (export "test_very_large") (result i64)
    i64.const 1000000
    i64.const 17
    i64.rem_u
  )

  ;; Test function for zero dividend
  (func (export "test_zero_dividend") (result i64)
    i64.const 0
    i64.const 5
    i64.rem_u
  )

  ;; Test function for unit divisor
  (func (export "test_unit_divisor") (result i64)
    i64.const 42
    i64.const 1
    i64.rem_u
  )

  ;; Test function for identity operation
  (func (export "test_identity") (result i64)
    i64.const 5
    i64.const 5
    i64.rem_u
  )

  ;; Test function for UINT64_MAX boundary (0xFFFFFFFFFFFFFFFF)
  (func (export "test_uint64_max_mod2") (result i64)
    i64.const 0xFFFFFFFFFFFFFFFF  ;; UINT64_MAX
    i64.const 2
    i64.rem_u
  )

  ;; Test function for UINT64_MAX boundary with 3
  (func (export "test_uint64_max_mod3") (result i64)
    i64.const 0xFFFFFFFFFFFFFFFF  ;; UINT64_MAX
    i64.const 3
    i64.rem_u
  )

  ;; Test function for UINT64_MAX-1 boundary
  (func (export "test_uint64_max_minus_1") (result i64)
    i64.const 0xFFFFFFFFFFFFFFFE  ;; UINT64_MAX-1
    i64.const 2
    i64.rem_u
  )

  ;; Test function for large unsigned value that would be negative if signed
  (func (export "test_large_unsigned") (result i64)
    i64.const 0x8000000000000000  ;; INT64_MAX + 1, large unsigned value
    i64.const 3
    i64.rem_u
  )

  ;; Test function for power of 2 divisor (optimized path)
  (func (export "test_power_of_two_8") (result i64)
    i64.const 15
    i64.const 8
    i64.rem_u
  )

  ;; Test function for power of 2 divisor (16)
  (func (export "test_power_of_two_16") (result i64)
    i64.const 31
    i64.const 16
    i64.rem_u
  )

  ;; Test function for power of 2 divisor (large: 2^32)
  (func (export "test_power_of_two_large") (result i64)
    i64.const 0xFFFFFFFFFFFFFFFF  ;; UINT64_MAX
    i64.const 0x100000000         ;; 2^32
    i64.rem_u
  )

  ;; Test function for small dividend with large divisor
  (func (export "test_small_div_large") (result i64)
    i64.const 100
    i64.const 0xFFFFFFFFFFFFFFFF  ;; UINT64_MAX
    i64.rem_u
  )

  ;; Test function for large numbers both as unsigned
  (func (export "test_both_large_unsigned") (result i64)
    i64.const 0xFFFFFFFFFFFFFFFE  ;; UINT64_MAX-1
    i64.const 0x8000000000000000  ;; INT64_MAX+1
    i64.rem_u
  )

  ;; Test function for consecutive large primes
  (func (export "test_large_prime") (result i64)
    i64.const 0xFFFFFFFFFFFFFFFF  ;; UINT64_MAX
    i64.const 2147483647          ;; Large prime (2^31 - 1)
    i64.rem_u
  )

  ;; Test function that should trap: division by zero
  ;; This function will cause a runtime trap when called
  (func (export "test_div_by_zero") (result i64)
    i64.const 42
    i64.const 0
    i64.rem_u
  )

  ;; Test function for zero dividend with zero divisor (should also trap)
  (func (export "test_zero_div_zero") (result i64)
    i64.const 0
    i64.const 0
    i64.rem_u
  )
)