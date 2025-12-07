(module
  ;; Basic i32.shr_s test function - takes two i32 parameters and returns shifted result
  (func (export "i32_shr_s_test") (param $value i32) (param $shift_count i32) (result i32)
    local.get $value
    local.get $shift_count
    i32.shr_s)

  ;; Single parameter test function for edge case testing
  (func (export "test_basic_shr_s") (param $input i32) (result i32)
    local.get $input
    i32.const 4  ;; Fixed shift count for basic testing
    i32.shr_s)

  ;; Zero shift test - should return original value (identity operation)
  (func (export "test_zero_shift") (param $input i32) (result i32)
    local.get $input
    i32.const 0
    i32.shr_s)

  ;; Maximum shift test - shift by 31 positions
  (func (export "test_max_shift") (param $input i32) (result i32)
    local.get $input
    i32.const 31
    i32.shr_s)

  ;; Modulo shift test - shift by 32 should wrap to shift by 0
  (func (export "test_modulo_shift") (param $input i32) (result i32)
    local.get $input
    i32.const 32
    i32.shr_s)

  ;; Large count shift test - test modulo behavior
  (func (export "test_large_shift") (param $input i32) (result i32)
    local.get $input
    i32.const 33  ;; Should be equivalent to shift by 1
    i32.shr_s)

  ;; Sign extension test - verify negative values maintain sign
  (func (export "test_sign_extension") (result i32)
    i32.const -8  ;; Negative value
    i32.const 1   ;; Shift by 1
    i32.shr_s)    ;; Should produce -4 (sign extended)

  ;; Boundary value test - INT32_MIN
  (func (export "test_int32_min") (result i32)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.const 1
    i32.shr_s)

  ;; Boundary value test - INT32_MAX
  (func (export "test_int32_max") (result i32)
    i32.const 0x7fffffff  ;; INT32_MAX (2147483647)
    i32.const 1
    i32.shr_s)

  ;; Powers of 2 test function - positive value
  (func (export "test_power_of_2_positive") (result i32)
    i32.const 64   ;; 2^6
    i32.const 2    ;; Shift by 2
    i32.shr_s)     ;; Should produce 16

  ;; Powers of 2 test function - negative value
  (func (export "test_power_of_2_negative") (result i32)
    i32.const -64  ;; Negative power of 2
    i32.const 2    ;; Shift by 2
    i32.shr_s)     ;; Should produce -16 (sign extended)

  ;; All ones test function
  (func (export "test_all_ones") (result i32)
    i32.const -1   ;; 0xffffffff
    i32.const 7
    i32.shr_s)     ;; Should remain -1 (all 1s with sign extension)

  ;; Alternating pattern test - positive
  (func (export "test_alternating_positive") (result i32)
    i32.const 0x55555555  ;; Alternating 01010101...
    i32.const 1
    i32.shr_s)

  ;; Alternating pattern test - negative
  (func (export "test_alternating_negative") (result i32)
    i32.const 0xaaaaaaaa  ;; Alternating 10101010... (negative)
    i32.const 1
    i32.shr_s)

  ;; Complex validation function for multiple test cases
  (func (export "validate_shr_s_properties") (result i32)
    (local $test_val i32)
    (local $temp i32)
    (local $success i32)

    ;; Initialize success flag
    i32.const 1
    local.set $success

    ;; Test value for property validation
    i32.const 0x12345678
    local.set $test_val

    ;; Property 1: shr_s(x, 0) == x (identity operation)
    local.get $test_val
    i32.const 0
    i32.shr_s
    local.get $test_val
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 2: shr_s(x, 32) == shr_s(x, 0) (modulo behavior)
    local.get $test_val
    i32.const 32
    i32.shr_s
    local.get $test_val
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 3: shr_s(0, any) == 0
    i32.const 0
    i32.const 25
    i32.shr_s
    i32.const 0
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 4: shr_s(-1, any_valid_shift) == -1 (sign extension)
    i32.const -1
    i32.const 13
    i32.shr_s
    i32.const -1
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 5: Test modulo behavior - shr_s(x, 33) == shr_s(x, 1)
    local.get $test_val
    i32.const 33
    i32.shr_s
    local.set $temp

    local.get $test_val
    i32.const 1
    i32.shr_s
    local.get $temp
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 6: Sign preservation for negative values
    i32.const -1000
    i32.const 5
    i32.shr_s
    i32.const 0
    i32.ge_s  ;; Should be negative (< 0)
    if
      i32.const 0
      local.set $success
    end

    ;; Property 7: Positive values remain non-negative
    i32.const 1000
    i32.const 5
    i32.shr_s
    i32.const 0
    i32.lt_s  ;; Should be non-negative (>= 0)
    if
      i32.const 0
      local.set $success
    end

    ;; Return success flag (1 if all tests pass, 0 if any fail)
    local.get $success)

  ;; Boundary extremes test function
  (func (export "test_boundary_extremes") (result i32)
    (local $success i32)

    ;; Initialize success flag
    i32.const 1
    local.set $success

    ;; Test 1: INT32_MIN >> 31 should produce -1
    i32.const 0x80000000  ;; INT32_MIN
    i32.const 31
    i32.shr_s
    i32.const -1
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 2: INT32_MAX >> 31 should produce 0
    i32.const 0x7fffffff  ;; INT32_MAX
    i32.const 31
    i32.shr_s
    i32.const 0
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 3: Large modulo shift (100 % 32 = 4)
    i32.const -16
    i32.const 100  ;; 100 % 32 = 4, so -16 >> 4 = -1
    i32.shr_s
    i32.const -1
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    local.get $success)

  ;; Performance test function for benchmarking
  (func (export "performance_test") (param $iterations i32) (result i32)
    (local $i i32)
    (local $result i32)

    i32.const 0
    local.set $i
    i32.const -12345678  ;; Start with negative value to test sign extension
    local.set $result

    (loop $loop
      local.get $result
      i32.const 1
      i32.shr_s
      local.set $result

      ;; Prevent result from becoming too small by adding back some value
      local.get $result
      i32.const -100000
      i32.gt_s
      if
        local.get $result
        i32.const 0x10000
        i32.or  ;; Add some bits back to maintain negative value
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

  ;; Mathematical property verification
  (func (export "verify_mathematical_properties") (result i32)
    (local $val i32)
    (local $success i32)

    i32.const 1
    local.set $success

    ;; Test consecutive shift property: (x >> a) >> b == x >> (a + b) when (a + b) < 32
    i32.const 1000
    local.set $val

    ;; Verify: (1000 >> 3) >> 2 == 1000 >> 5
    local.get $val
    i32.const 3
    i32.shr_s
    i32.const 2
    i32.shr_s  ;; (1000 >> 3) >> 2

    local.get $val
    i32.const 5
    i32.shr_s  ;; 1000 >> 5

    i32.ne
    if
      i32.const 0
      local.set $success
    end

    local.get $success)
)