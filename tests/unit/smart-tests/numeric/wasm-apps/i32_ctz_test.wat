(module
  ;; Basic i32.ctz test function
  (func (export "test_basic_ctz") (param $input i32) (result i32)
    local.get $input
    i32.ctz)

  ;; Comprehensive test function that validates multiple values
  (func (export "test_comprehensive_ctz") (result i32)
    (local $result i32)
    (local $success i32)

    ;; Initialize success flag
    i32.const 1
    local.set $success

    ;; Test 1: i32.ctz(0) should return 32
    i32.const 0
    i32.ctz
    i32.const 32
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 2: i32.ctz(1) should return 0
    i32.const 1
    i32.ctz
    i32.const 0
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 3: i32.ctz(8) should return 3
    i32.const 8
    i32.ctz
    i32.const 3
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 4: i32.ctz(0x80000000) should return 31
    i32.const 0x80000000
    i32.ctz
    i32.const 31
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 5: i32.ctz(0xFFFFFFFF) should return 0
    i32.const 0xFFFFFFFF
    i32.ctz
    i32.const 0
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Return success flag
    local.get $success)

  ;; Powers of 2 test function
  (func (export "test_powers_of_2") (result i32)
    (local $i i32)
    (local $value i32)
    (local $expected i32)
    (local $actual i32)

    ;; Test powers of 2 from 2^0 to 2^10
    i32.const 0
    local.set $i

    loop $test_loop
      ;; Calculate 2^i
      i32.const 1
      local.get $i
      i32.shl
      local.set $value

      ;; Calculate i32.ctz(2^i)
      local.get $value
      i32.ctz
      local.set $actual

      ;; Expected result should be i
      local.get $i
      local.set $expected

      ;; Check if actual == expected
      local.get $actual
      local.get $expected
      i32.ne
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

      ;; Continue if i <= 10
      local.get $i
      i32.const 11
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

    ;; Test 0xAAAAAAAA (10101010...) should return 1
    i32.const 0xAAAAAAAA
    i32.ctz
    i32.const 1
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0x55555555 (01010101...) should return 0
    i32.const 0x55555555
    i32.ctz
    i32.const 0
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0xFF000000 should return 24
    i32.const 0xFF000000
    i32.ctz
    i32.const 24
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0xF0000000 should return 28
    i32.const 0xF0000000
    i32.ctz
    i32.const 28
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    local.get $success)

  ;; Single bit position tests
  (func (export "test_single_bits") (result i32)
    (local $bit_pos i32)
    (local $value i32)
    (local $result i32)

    ;; Test individual bit positions
    i32.const 0
    local.set $bit_pos

    loop $bit_loop
      ;; Calculate value with bit at position bit_pos set
      i32.const 1
      local.get $bit_pos
      i32.shl
      local.set $value

      ;; Calculate i32.ctz for this value
      local.get $value
      i32.ctz
      local.set $result

      ;; Result should equal bit_pos
      local.get $result
      local.get $bit_pos
      i32.ne
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

      ;; Continue for positions 0-31
      local.get $bit_pos
      i32.const 32
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
    i32.const 0
    i32.ctz
    i32.const 32
    i32.ne
    if
      i32.const 0
      return
    end

    ;; Test maximum signed positive (0x7FFFFFFF)
    i32.const 0x7FFFFFFF
    i32.ctz
    i32.const 0
    i32.ne
    if
      i32.const 0
      return
    end

    ;; Test maximum unsigned (0xFFFFFFFF)
    i32.const 0xFFFFFFFF
    i32.ctz
    i32.const 0
    i32.ne
    if
      i32.const 0
      return
    end

    ;; Test minimum signed (0x80000000)
    i32.const 0x80000000
    i32.ctz
    i32.const 31
    i32.ne
    if
      i32.const 0
      return
    end

    local.get $success)

  ;; Performance test for multiple operations
  (func (export "test_performance") (result i32)
    (local $i i32)
    (local $sum i32)

    ;; Perform 1000 i32.ctz operations to test performance
    i32.const 0
    local.set $i
    i32.const 0
    local.set $sum

    loop $perf_loop
      local.get $i
      i32.ctz
      local.get $sum
      i32.add
      local.set $sum

      local.get $i
      i32.const 1
      i32.add
      local.set $i

      local.get $i
      i32.const 1000
      i32.lt_s
      br_if $perf_loop
    end

    ;; Return accumulated sum as performance indicator
    local.get $sum)
)