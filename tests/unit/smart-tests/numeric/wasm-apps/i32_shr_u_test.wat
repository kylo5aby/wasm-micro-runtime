(module
  ;; Basic i32.shr_u test function - takes two i32 parameters and returns shifted result
  (func (export "i32_shr_u_test") (param $value i32) (param $shift_count i32) (result i32)
    local.get $value
    local.get $shift_count
    i32.shr_u)

  ;; Single parameter test function for edge case testing
  (func (export "test_basic_shr_u") (param $input i32) (result i32)
    local.get $input
    i32.const 4  ;; Fixed shift count for basic testing
    i32.shr_u)

  ;; Zero shift test - should return original value (identity operation)
  (func (export "test_zero_shift") (param $input i32) (result i32)
    local.get $input
    i32.const 0
    i32.shr_u)

  ;; Maximum shift test - shift by 31 positions
  (func (export "test_max_shift") (param $input i32) (result i32)
    local.get $input
    i32.const 31
    i32.shr_u)

  ;; Modulo shift test - shift by 32 should wrap to shift by 0
  (func (export "test_modulo_shift") (param $input i32) (result i32)
    local.get $input
    i32.const 32
    i32.shr_u)

  ;; Large count shift test - test modulo behavior
  (func (export "test_large_shift") (param $input i32) (result i32)
    local.get $input
    i32.const 33  ;; Should be equivalent to shift by 1
    i32.shr_u)

  ;; Zero fill test - verify unsigned behavior (no sign extension)
  (func (export "test_zero_fill") (result i32)
    i32.const 0x80000000  ;; High bit set (negative as signed)
    i32.const 1           ;; Shift by 1
    i32.shr_u)            ;; Should produce 0x40000000 (zero-filled)

  ;; Boundary value test - UINT32_MAX
  (func (export "test_uint32_max") (result i32)
    i32.const 0xffffffff  ;; UINT32_MAX (all bits set)
    i32.const 1
    i32.shr_u)

  ;; Boundary value test - INT32_MIN as unsigned
  (func (export "test_int32_min_unsigned") (result i32)
    i32.const 0x80000000  ;; INT32_MIN as unsigned value
    i32.const 1
    i32.shr_u)

  ;; Powers of 2 test function - positive value
  (func (export "test_power_of_2") (result i32)
    i32.const 64   ;; 2^6
    i32.const 2    ;; Shift by 2
    i32.shr_u)     ;; Should produce 16

  ;; Powers of 2 test function - high bit set
  (func (export "test_power_of_2_high_bit") (result i32)
    i32.const 0x80000000  ;; 2^31 (high bit)
    i32.const 2           ;; Shift by 2
    i32.shr_u)            ;; Should produce 0x20000000

  ;; All ones test function
  (func (export "test_all_ones") (result i32)
    i32.const 0xffffffff  ;; All bits set
    i32.const 7
    i32.shr_u)            ;; Should produce 0x01ffffff (zero-filled)

  ;; Alternating pattern test - positive bits
  (func (export "test_alternating_positive") (result i32)
    i32.const 0x55555555  ;; Alternating 01010101...
    i32.const 1
    i32.shr_u)

  ;; Alternating pattern test - high bits set
  (func (export "test_alternating_high_bits") (result i32)
    i32.const 0xaaaaaaaa  ;; Alternating 10101010...
    i32.const 1
    i32.shr_u)

  ;; Complex validation function for multiple test cases
  (func (export "validate_shr_u_properties") (result i32)
    (local $test_val i32)
    (local $temp i32)
    (local $success i32)

    ;; Initialize success flag
    i32.const 1
    local.set $success

    ;; Test value for property validation
    i32.const 0x12345678
    local.set $test_val

    ;; Property 1: shr_u(x, 0) == x (identity operation)
    local.get $test_val
    i32.const 0
    i32.shr_u
    local.get $test_val
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 2: shr_u(x, 32) == shr_u(x, 0) (modulo behavior)
    local.get $test_val
    i32.const 32
    i32.shr_u
    local.get $test_val
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 3: shr_u(0, any) == 0
    i32.const 0
    i32.const 25
    i32.shr_u
    i32.const 0
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 4: shr_u(0xffffffff, 1) == 0x7fffffff (zero-fill, not sign extend)
    i32.const 0xffffffff
    i32.const 1
    i32.shr_u
    i32.const 0x7fffffff
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 5: Test modulo behavior - shr_u(x, 33) == shr_u(x, 1)
    local.get $test_val
    i32.const 33
    i32.shr_u
    local.set $temp

    local.get $test_val
    i32.const 1
    i32.shr_u
    local.get $temp
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 6: Zero-fill behavior for high bit set
    i32.const 0x80000000
    i32.const 1
    i32.shr_u
    i32.const 0x40000000
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 7: Maximum shift produces 0 or 1
    i32.const 0x80000000  ;; High bit set
    i32.const 31          ;; Maximum shift
    i32.shr_u
    i32.const 1           ;; Should produce 1
    i32.ne
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

    ;; Test 1: 0x80000000 >> 31 should produce 1 (not -1 like signed)
    i32.const 0x80000000  ;; High bit set
    i32.const 31
    i32.shr_u
    i32.const 1
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 2: 0x7fffffff >> 31 should produce 0
    i32.const 0x7fffffff  ;; Maximum positive value
    i32.const 31
    i32.shr_u
    i32.const 0
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 3: Large modulo shift (100 % 32 = 4)
    i32.const 0xffffffff  ;; All bits set
    i32.const 100         ;; 100 % 32 = 4, so 0xffffffff >> 4 = 0x0fffffff
    i32.shr_u
    i32.const 0x0fffffff
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
    i32.const 0xffffffff  ;; Start with all bits set
    local.set $result

    (loop $loop
      local.get $result
      i32.const 1
      i32.shr_u
      local.set $result

      ;; Prevent result from becoming 0 by adding back some value
      local.get $result
      i32.const 0x1000
      i32.lt_u
      if
        local.get $result
        i32.const 0xf0000000
        i32.or  ;; Add high bits back
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
    i32.const 0x12345678
    local.set $val

    ;; Verify: (0x12345678 >> 3) >> 2 == 0x12345678 >> 5
    local.get $val
    i32.const 3
    i32.shr_u
    i32.const 2
    i32.shr_u  ;; (val >> 3) >> 2

    local.get $val
    i32.const 5
    i32.shr_u  ;; val >> 5

    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test division property: x >> n == x / (2^n) for unsigned values
    i32.const 1000
    local.set $val

    ;; Verify: 1000 >> 3 == 1000 / 8
    local.get $val
    i32.const 3
    i32.shr_u  ;; 1000 >> 3

    local.get $val
    i32.const 8
    i32.div_u  ;; 1000 / 8

    i32.ne
    if
      i32.const 0
      local.set $success
    end

    local.get $success)

  ;; Stack underflow test function - this should cause a trap when called with insufficient stack
  (func (export "test_stack_underflow") (result i32)
    ;; This function attempts to use i32.shr_u without having 2 values on stack
    ;; Note: This is invalid WAT and should be caught during validation/loading
    i32.const 42
    ;; Missing second operand - this will cause module validation to fail
    ;; i32.shr_u  ;; Commented out as it would prevent module loading
    )
)