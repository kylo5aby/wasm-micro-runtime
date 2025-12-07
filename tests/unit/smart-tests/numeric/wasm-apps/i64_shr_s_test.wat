(module
  ;; Basic i64.shr_s test function - takes two i64 parameters and returns shifted result
  (func (export "i64_shr_s_test") (param $value i64) (param $shift_count i64) (result i64)
    local.get $value
    local.get $shift_count
    i64.shr_s)

  ;; Single parameter test function for edge case testing
  (func (export "test_basic_shr_s") (param $input i64) (result i64)
    local.get $input
    i64.const 4  ;; Fixed shift count for basic testing
    i64.shr_s)

  ;; Zero shift test - should return original value (identity operation)
  (func (export "test_zero_shift") (param $input i64) (result i64)
    local.get $input
    i64.const 0
    i64.shr_s)

  ;; Maximum shift test - shift by 63 positions (64-bit max)
  (func (export "test_max_shift") (param $input i64) (result i64)
    local.get $input
    i64.const 63
    i64.shr_s)

  ;; Modulo shift test - shift by 64 should wrap to shift by 0
  (func (export "test_modulo_shift") (param $input i64) (result i64)
    local.get $input
    i64.const 64
    i64.shr_s)

  ;; Large count shift test - test modulo 64 behavior
  (func (export "test_large_shift") (param $input i64) (result i64)
    local.get $input
    i64.const 65  ;; Should be equivalent to shift by 1
    i64.shr_s)

  ;; Sign extension test - verify negative 64-bit values maintain sign
  (func (export "test_sign_extension") (result i64)
    i64.const -8  ;; Negative value
    i64.const 1   ;; Shift by 1
    i64.shr_s)    ;; Should produce -4 (sign extended)

  ;; Boundary value test - INT64_MIN
  (func (export "test_int64_min") (result i64)
    i64.const 0x8000000000000000  ;; INT64_MIN (-9223372036854775808)
    i64.const 1
    i64.shr_s)

  ;; Boundary value test - INT64_MAX
  (func (export "test_int64_max") (result i64)
    i64.const 0x7fffffffffffffff  ;; INT64_MAX (9223372036854775807)
    i64.const 1
    i64.shr_s)

  ;; Powers of 2 test function - positive 64-bit value
  (func (export "test_power_of_2_positive") (result i64)
    i64.const 256   ;; 2^8
    i64.const 2     ;; Shift by 2
    i64.shr_s)      ;; Should produce 64

  ;; Powers of 2 test function - negative 64-bit value
  (func (export "test_power_of_2_negative") (result i64)
    i64.const -256  ;; Negative power of 2
    i64.const 2     ;; Shift by 2
    i64.shr_s)      ;; Should produce -64 (sign extended)

  ;; All ones test function - 64-bit
  (func (export "test_all_ones") (result i64)
    i64.const -1   ;; 0xffffffffffffffff
    i64.const 7
    i64.shr_s)     ;; Should remain -1 (all 1s with sign extension)

  ;; Alternating pattern test - positive 64-bit
  (func (export "test_alternating_positive") (result i64)
    i64.const 0x5555555555555555  ;; Alternating 64-bit pattern
    i64.const 1
    i64.shr_s)

  ;; Alternating pattern test - negative 64-bit
  (func (export "test_alternating_negative") (result i64)
    i64.const 0xaaaaaaaaaaaaaaaa  ;; Alternating negative 64-bit pattern
    i64.const 1
    i64.shr_s)

  ;; Large 64-bit value test
  (func (export "test_large_64bit_value") (result i64)
    i64.const 0x123456789abcdef0  ;; Large 64-bit value
    i64.const 4
    i64.shr_s)

  ;; Upper 32-bit range test
  (func (export "test_upper_32bit_range") (result i64)
    i64.const 0x100000000  ;; 2^32
    i64.const 1
    i64.shr_s)

  ;; Complex validation function for multiple 64-bit test cases
  (func (export "validate_shr_s_properties") (result i32)
    (local $test_val i64)
    (local $temp i64)
    (local $success i32)

    ;; Initialize success flag
    i32.const 1
    local.set $success

    ;; Test value for property validation (64-bit)
    i64.const 0x123456789abcdef0
    local.set $test_val

    ;; Property 1: shr_s(x, 0) == x (identity operation)
    local.get $test_val
    i64.const 0
    i64.shr_s
    local.get $test_val
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 2: shr_s(x, 64) == shr_s(x, 0) (modulo 64 behavior)
    local.get $test_val
    i64.const 64
    i64.shr_s
    local.get $test_val
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 3: shr_s(0, any) == 0
    i64.const 0
    i64.const 25
    i64.shr_s
    i64.const 0
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 4: shr_s(-1, any_valid_shift) == -1 (sign extension)
    i64.const -1
    i64.const 13
    i64.shr_s
    i64.const -1
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 5: Test modulo behavior - shr_s(x, 65) == shr_s(x, 1)
    local.get $test_val
    i64.const 65
    i64.shr_s
    local.set $temp

    local.get $test_val
    i64.const 1
    i64.shr_s
    local.get $temp
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 6: Sign preservation for negative values
    i64.const -1000000
    i64.const 5
    i64.shr_s
    i64.const 0
    i64.ge_s  ;; Should be negative (< 0)
    if
      i32.const 0
      local.set $success
    end

    ;; Property 7: Positive values remain non-negative
    i64.const 1000000
    i64.const 5
    i64.shr_s
    i64.const 0
    i64.lt_s  ;; Should be non-negative (>= 0)
    if
      i32.const 0
      local.set $success
    end

    ;; Return success flag (1 if all tests pass, 0 if any fail)
    local.get $success)

  ;; Boundary extremes test function for 64-bit
  (func (export "test_boundary_extremes") (result i32)
    (local $success i32)

    ;; Initialize success flag
    i32.const 1
    local.set $success

    ;; Test 1: INT64_MIN >> 63 should produce -1
    i64.const 0x8000000000000000  ;; INT64_MIN
    i64.const 63
    i64.shr_s
    i64.const -1
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 2: INT64_MAX >> 63 should produce 0
    i64.const 0x7fffffffffffffff  ;; INT64_MAX
    i64.const 63
    i64.shr_s
    i64.const 0
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 3: Large modulo shift (100 % 64 = 36)
    i64.const -256
    i64.const 100  ;; 100 % 64 = 36, so -256 >> 36 = -1
    i64.shr_s
    i64.const -1
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 4: High-order bit pattern preservation
    i64.const 0xff00000000000000  ;; High byte set (negative)
    i64.const 8
    i64.shr_s
    i64.const 0xffff000000000000  ;; Should preserve sign bits
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    local.get $success)

  ;; Performance test function for 64-bit benchmarking
  (func (export "performance_test") (param $iterations i32) (result i64)
    (local $i i32)
    (local $result i64)

    i32.const 0
    local.set $i
    i64.const -123456789012345678  ;; Large negative 64-bit value
    local.set $result

    (loop $loop
      local.get $result
      i64.const 1
      i64.shr_s
      local.set $result

      ;; Prevent result from becoming too small by adding back some value
      local.get $result
      i64.const -1000000
      i64.gt_s
      if
        local.get $result
        i64.const 0x100000000  ;; Add back some bits to maintain value
        i64.or
        local.set $result
      end

      local.get $i
      i32.const 1
      i32.add
      local.set $i

      local.get $i
      local.get $iterations
      i32.lt_u
      br_if $loop)

    local.get $result)

  ;; Mathematical property verification for 64-bit
  (func (export "verify_mathematical_properties") (result i32)
    (local $val i64)
    (local $success i32)

    i32.const 1
    local.set $success

    ;; Test consecutive shift property: (x >> a) >> b == x >> (a + b) when (a + b) < 64
    i64.const 1000000
    local.set $val

    ;; Verify: (1000000 >> 3) >> 2 == 1000000 >> 5
    local.get $val
    i64.const 3
    i64.shr_s
    i64.const 2
    i64.shr_s  ;; (1000000 >> 3) >> 2

    local.get $val
    i64.const 5
    i64.shr_s  ;; 1000000 >> 5

    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test large shift equivalence: verify modulo 64 behavior
    i64.const 0x123456789abcdef0
    i64.const 128  ;; 128 % 64 = 0
    i64.shr_s

    i64.const 0x123456789abcdef0
    i64.const 0
    i64.shr_s

    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test sign preservation through multiple shifts
    i64.const -1000000000
    i64.const 10
    i64.shr_s
    i64.const 0
    i64.ge_s  ;; Result should still be negative
    if
      i32.const 0
      local.set $success
    end

    local.get $success)

  ;; Stack underflow test functions (these should cause validation errors)
  ;; These functions are intentionally designed to test error conditions

  ;; Test insufficient stack operands - single operand (should fail validation)
  (func (export "test_stack_underflow_single") (result i64)
    i64.const 42
    ;; Missing second operand - this should cause module validation to fail
    ;; i64.shr_s  ;; Commented out to prevent validation error during loading
    )

  ;; Test empty stack operation (should fail validation)
  (func (export "test_stack_underflow_empty") (result i64)
    ;; No operands on stack - this should cause module validation to fail
    ;; i64.shr_s  ;; Commented out to prevent validation error during loading
    i64.const 0  ;; Return dummy value
    )

  ;; Division equivalence test for positive values
  (func (export "test_division_equivalence") (result i32)
    (local $success i32)
    (local $shift_result i64)
    (local $div_result i64)

    i32.const 1
    local.set $success

    ;; Test: 1024 >> 2 should equal 1024 / 4
    i64.const 1024
    i64.const 2
    i64.shr_s
    local.set $shift_result

    i64.const 1024
    i64.const 4
    i64.div_s
    local.set $div_result

    local.get $shift_result
    local.get $div_result
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test: 2048 >> 3 should equal 2048 / 8
    i64.const 2048
    i64.const 3
    i64.shr_s
    local.set $shift_result

    i64.const 2048
    i64.const 8
    i64.div_s
    local.set $div_result

    local.get $shift_result
    local.get $div_result
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    local.get $success)
)