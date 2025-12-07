(module
  ;; Basic i64.ctz test function
  (func (export "test_i64_ctz") (param $input i64) (result i64)
    local.get $input
    i64.ctz)

  ;; Comprehensive test function that validates multiple values
  (func (export "test_comprehensive_i64_ctz") (result i32)
    (local $result i64)
    (local $success i32)

    ;; Initialize success flag
    i32.const 1
    local.set $success

    ;; Test 1: i64.ctz(0) should return 64
    i64.const 0
    i64.ctz
    i64.const 64
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 2: i64.ctz(1) should return 0
    i64.const 1
    i64.ctz
    i64.const 0
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 3: i64.ctz(8) should return 3
    i64.const 8
    i64.ctz
    i64.const 3
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 4: i64.ctz(0x8000000000000000) should return 63
    i64.const 0x8000000000000000
    i64.ctz
    i64.const 63
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 5: i64.ctz(0xFFFFFFFFFFFFFFFF) should return 0
    i64.const 0xFFFFFFFFFFFFFFFF
    i64.ctz
    i64.const 0
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Return success flag
    local.get $success)

  ;; Powers of 2 test function - tests 2^0 through 2^63
  (func (export "test_powers_of_2") (result i32)
    (local $i i32)
    (local $value i64)
    (local $expected i64)
    (local $actual i64)

    ;; Test powers of 2 from 2^0 to 2^63
    i32.const 0
    local.set $i

    loop $test_loop
      ;; Calculate 2^i
      i64.const 1
      local.get $i
      i64.extend_i32_u
      i64.shl
      local.set $value

      ;; Calculate i64.ctz(2^i)
      local.get $value
      i64.ctz
      local.set $actual

      ;; Expected result should be i
      local.get $i
      i64.extend_i32_u
      local.set $expected

      ;; Check if actual == expected
      local.get $actual
      local.get $expected
      i64.ne
      if
        ;; Return error code (negative of failed index)
        local.get $i
        i32.const -1
        i32.mul
        return
      end

      ;; Increment counter
      local.get $i
      i32.const 1
      i32.add
      local.set $i

      ;; Continue if i <= 63
      local.get $i
      i32.const 64
      i32.lt_s
      br_if $test_loop
    end

    ;; All tests passed, return success
    i32.const 1)

  ;; Alternating bit pattern tests
  (func (export "test_alternating_patterns") (result i32)
    (local $success i32)

    ;; Initialize success
    i32.const 1
    local.set $success

    ;; Test 0xAAAAAAAAAAAAAAAA (10101010...) should return 1
    i64.const 0xAAAAAAAAAAAAAAAA
    i64.ctz
    i64.const 1
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0x5555555555555555 (01010101...) should return 0
    i64.const 0x5555555555555555
    i64.ctz
    i64.const 0
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0xFFFFFFFFFFFFF000 should return 12
    i64.const 0xFFFFFFFFFFFFF000
    i64.ctz
    i64.const 12
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0xFFFFFFFFFF000000 should return 24
    i64.const 0xFFFFFFFFFF000000
    i64.ctz
    i64.const 24
    i64.ne
    if
      i32.const 0
      local.set $success
    end

    local.get $success)

  ;; Single bit position tests
  (func (export "test_single_bits") (result i32)
    (local $bit_pos i32)
    (local $value i64)
    (local $result i64)

    ;; Test individual bit positions
    i32.const 0
    local.set $bit_pos

    loop $bit_loop
      ;; Calculate value with bit at position bit_pos set
      i64.const 1
      local.get $bit_pos
      i64.extend_i32_u
      i64.shl
      local.set $value

      ;; Calculate i64.ctz for this value
      local.get $value
      i64.ctz
      local.set $result

      ;; Result should equal bit_pos
      local.get $result
      local.get $bit_pos
      i64.extend_i32_u
      i64.ne
      if
        ;; Return negative bit position as error
        local.get $bit_pos
        i32.const -1
        i32.mul
        return
      end

      ;; Increment bit position
      local.get $bit_pos
      i32.const 1
      i32.add
      local.set $bit_pos

      ;; Continue for positions 0-63
      local.get $bit_pos
      i32.const 64
      i32.lt_s
      br_if $bit_loop
    end

    ;; All tests passed
    i32.const 1)

  ;; Boundary value tests
  (func (export "test_boundaries") (result i32)
    (local $success i32)

    i32.const 1
    local.set $success

    ;; Test minimum value (0)
    i64.const 0
    i64.ctz
    i64.const 64
    i64.ne
    if
      i32.const 0
      return
    end

    ;; Test maximum signed positive (0x7FFFFFFFFFFFFFFF)
    i64.const 0x7FFFFFFFFFFFFFFF
    i64.ctz
    i64.const 0
    i64.ne
    if
      i32.const 0
      return
    end

    ;; Test maximum unsigned (0xFFFFFFFFFFFFFFFF)
    i64.const 0xFFFFFFFFFFFFFFFF
    i64.ctz
    i64.const 0
    i64.ne
    if
      i32.const 0
      return
    end

    ;; Test minimum signed (0x8000000000000000)
    i64.const 0x8000000000000000
    i64.ctz
    i64.const 63
    i64.ne
    if
      i32.const 0
      return
    end

    local.get $success)

  ;; Performance test for multiple operations
  (func (export "test_performance") (result i64)
    (local $i i64)
    (local $sum i64)

    ;; Perform 1000 i64.ctz operations to test performance
    i64.const 0
    local.set $i
    i64.const 0
    local.set $sum

    loop $perf_loop
      local.get $i
      i64.ctz
      local.get $sum
      i64.add
      local.set $sum

      local.get $i
      i64.const 1
      i64.add
      local.set $i

      local.get $i
      i64.const 1000
      i64.lt_s
      br_if $perf_loop
    end

    ;; Return accumulated sum as performance indicator
    local.get $sum)
)