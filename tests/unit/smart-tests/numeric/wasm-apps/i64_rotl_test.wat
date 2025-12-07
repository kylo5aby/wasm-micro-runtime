(module
  ;; Basic i64.rotl test function - takes two i64 parameters and returns rotated result
  (func (export "i64_rotl_test") (param $value i64) (param $count i64) (result i64)
    local.get $value
    local.get $count
    i64.rotl)

  ;; Single parameter test function for edge case testing
  (func (export "test_basic_rotl") (param $input i64) (result i64)
    local.get $input
    i64.const 4  ;; Fixed rotation count for basic testing
    i64.rotl)

  ;; Zero rotation test - should return original value
  (func (export "test_zero_rotation") (param $input i64) (result i64)
    local.get $input
    i64.const 0
    i64.rotl)

  ;; Full rotation test - rotate by 64 should return original value
  (func (export "test_full_rotation") (param $input i64) (result i64)
    local.get $input
    i64.const 64
    i64.rotl)

  ;; Boundary rotation test - rotate by 63
  (func (export "test_boundary_rotation") (param $input i64) (result i64)
    local.get $input
    i64.const 63
    i64.rotl)

  ;; Large count rotation test - test modulo behavior
  (func (export "test_large_count") (param $input i64) (result i64)
    local.get $input
    i64.const 65  ;; Should be equivalent to rotation by 1
    i64.rotl)

  ;; Mathematical property test - rotate twice to verify consistency
  (func (export "test_double_rotation") (param $value i64) (param $count1 i64) (param $count2 i64) (result i64)
    local.get $value
    local.get $count1
    i64.rotl
    local.get $count2
    i64.rotl)

  ;; Powers of 2 test function
  (func (export "test_power_of_2") (result i64)
    i64.const 1    ;; Test rotating single bit
    i64.const 1
    i64.rotl)

  ;; All ones test function
  (func (export "test_all_ones") (result i64)
    i64.const 0xffffffffffffffff
    i64.const 7
    i64.rotl)

  ;; Alternating pattern test
  (func (export "test_alternating_pattern") (result i64)
    i64.const 0x5555555555555555
    i64.const 1
    i64.rotl)

  ;; MSB test function - rotate MSB to LSB
  (func (export "test_msb_rotation") (result i64)
    i64.const 0x8000000000000000
    i64.const 1
    i64.rotl)

  ;; LSB test function - rotate LSB upward
  (func (export "test_lsb_rotation") (result i64)
    i64.const 0x0000000000000001
    i64.const 8
    i64.rotl)

  ;; Complex validation function for multiple test cases
  (func (export "validate_rotl_properties") (result i64)
    (local $test_val i64)
    (local $temp i64)
    (local $success i64)

    ;; Initialize success flag
    i64.const 1
    local.set $success

    ;; Test value for property validation
    i64.const 0x123456789abcdef0
    local.set $test_val

    ;; Property 1: rotl(x, 0) == x
    local.get $test_val
    i64.const 0
    i64.rotl
    local.get $test_val
    i64.ne
    if
      i64.const 0
      local.set $success
    end

    ;; Property 2: rotl(x, 64) == x
    local.get $test_val
    i64.const 64
    i64.rotl
    local.get $test_val
    i64.ne
    if
      i64.const 0
      local.set $success
    end

    ;; Property 3: rotl(0xffffffffffffffff, any) == 0xffffffffffffffff
    i64.const 0xffffffffffffffff
    i64.const 13
    i64.rotl
    i64.const 0xffffffffffffffff
    i64.ne
    if
      i64.const 0
      local.set $success
    end

    ;; Property 4: rotl(0, any) == 0
    i64.const 0
    i64.const 25
    i64.rotl
    i64.const 0
    i64.ne
    if
      i64.const 0
      local.set $success
    end

    ;; Property 5: Test modulo behavior - rotl(x, 65) == rotl(x, 1)
    local.get $test_val
    i64.const 65
    i64.rotl
    local.set $temp

    local.get $test_val
    i64.const 1
    i64.rotl
    local.get $temp
    i64.ne
    if
      i64.const 0
      local.set $success
    end

    ;; Property 6: Test large modulo - rotl(x, 68) == rotl(x, 4)
    local.get $test_val
    i64.const 68
    i64.rotl
    local.set $temp

    local.get $test_val
    i64.const 4
    i64.rotl
    local.get $temp
    i64.ne
    if
      i64.const 0
      local.set $success
    end

    ;; Return success flag (1 if all tests pass, 0 if any fail)
    local.get $success)

  ;; Performance test function for benchmarking
  (func (export "performance_test") (param $iterations i32) (result i64)
    (local $i i32)
    (local $result i64)

    i32.const 0
    local.set $i
    i64.const 0x123456789abcdef0
    local.set $result

    (loop $loop
      local.get $result
      i64.const 7
      i64.rotl
      local.set $result

      local.get $i
      i32.const 1
      i32.add
      local.set $i

      local.get $i
      local.get $iterations
      i32.lt_u
      br_if $loop)

    local.get $result)

  ;; Extreme value test functions
  (func (export "test_max_value") (result i64)
    i64.const 0xffffffffffffffff
    i64.const 1
    i64.rotl)

  (func (export "test_min_value") (result i64)
    i64.const 0
    i64.const 32
    i64.rotl)

  ;; Bit pattern preservation test
  (func (export "test_bit_pattern") (param $pattern i64) (param $rotations i64) (result i64)
    local.get $pattern
    local.get $rotations
    i64.rotl
    local.get $rotations
    i64.const -1
    i64.mul  ;; Negate rotation count
    i64.const 64
    i64.and  ;; Convert to equivalent right rotation
    i64.rotr)

  ;; Chain rotation test - multiple sequential rotations
  (func (export "test_chain_rotation") (param $value i64) (result i64)
    local.get $value
    i64.const 8
    i64.rotl
    i64.const 8
    i64.rotl
    i64.const 8
    i64.rotl
    i64.const 8
    i64.rotl)  ;; Total: 32-bit rotation
)