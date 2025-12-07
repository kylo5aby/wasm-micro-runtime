(module
  ;; Basic i32.popcnt test function
  (func (export "test_basic_popcnt") (param $input i32) (result i32)
    local.get $input
    i32.popcnt)

  ;; Comprehensive test function that validates multiple values
  (func (export "test_comprehensive_popcnt") (result i32)
    (local $result i32)
    (local $success i32)

    ;; Initialize success flag
    i32.const 1
    local.set $success

    ;; Test 1: i32.popcnt(0) should return 0
    i32.const 0
    i32.popcnt
    i32.const 0
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 2: i32.popcnt(1) should return 1
    i32.const 1
    i32.popcnt
    i32.const 1
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 3: i32.popcnt(15) should return 4 (binary: 1111)
    i32.const 15
    i32.popcnt
    i32.const 4
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 4: i32.popcnt(0xFFFFFFFF) should return 32
    i32.const 0xFFFFFFFF
    i32.popcnt
    i32.const 32
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 5: i32.popcnt(0x80000000) should return 1
    i32.const 0x80000000
    i32.popcnt
    i32.const 1
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
    (local $actual i32)

    ;; Test powers of 2 from 2^0 to 2^10 - all should have popcnt = 1
    i32.const 0
    local.set $i

    loop $test_loop
      ;; Calculate 2^i
      i32.const 1
      local.get $i
      i32.shl
      local.set $value

      ;; Calculate i32.popcnt(2^i)
      local.get $value
      i32.popcnt
      local.set $actual

      ;; Expected result should always be 1 for powers of 2
      local.get $actual
      i32.const 1
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

    ;; Test 0xAAAAAAAA (10101010...) should return 16
    i32.const 0xAAAAAAAA
    i32.popcnt
    i32.const 16
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0x55555555 (01010101...) should return 16
    i32.const 0x55555555
    i32.popcnt
    i32.const 16
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0xFF000000 should return 8 (8 bits set in upper byte)
    i32.const 0xFF000000
    i32.popcnt
    i32.const 8
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0xF0F0F0F0 should return 16 (4 bits per nibble, 4 nibbles)
    i32.const 0xF0F0F0F0
    i32.popcnt
    i32.const 16
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    local.get $success)

  ;; Boundary value tests
  (func (export "test_boundaries") (result i32)
    (local $success i32)

    i32.const 1
    local.set $success

    ;; Test minimum value (0) - 0 bits set
    i32.const 0
    i32.popcnt
    i32.const 0
    i32.ne
    if
      i32.const 0
      return
    end

    ;; Test maximum signed positive (0x7FFFFFFF) - 31 bits set
    i32.const 0x7FFFFFFF
    i32.popcnt
    i32.const 31
    i32.ne
    if
      i32.const 0
      return
    end

    ;; Test maximum unsigned (0xFFFFFFFF) - 32 bits set
    i32.const 0xFFFFFFFF
    i32.popcnt
    i32.const 32
    i32.ne
    if
      i32.const 0
      return
    end

    ;; Test minimum signed (0x80000000) - 1 bit set (MSB)
    i32.const 0x80000000
    i32.popcnt
    i32.const 1
    i32.ne
    if
      i32.const 0
      return
    end

    local.get $success)

  ;; Standard integer values test
  (func (export "test_standard_integers") (result i32)
    (local $success i32)

    i32.const 1
    local.set $success

    ;; Test 7 (binary: 111) - 3 bits set
    i32.const 7
    i32.popcnt
    i32.const 3
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 31 (binary: 11111) - 5 bits set
    i32.const 31
    i32.popcnt
    i32.const 5
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 42 (binary: 101010) - 3 bits set
    i32.const 42
    i32.popcnt
    i32.const 3
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 255 (binary: 11111111) - 8 bits set
    i32.const 255
    i32.popcnt
    i32.const 8
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    local.get $success)

  ;; Mathematical property test: popcnt(x) + popcnt(~x) = 32
  (func (export "test_complement_property") (result i32)
    (local $value i32)
    (local $complement i32)
    (local $popcnt_val i32)
    (local $popcnt_comp i32)
    (local $sum i32)

    ;; Test with value 0xF0F0F0F0
    i32.const 0xF0F0F0F0
    local.set $value

    ;; Calculate complement (~value)
    local.get $value
    i32.const -1
    i32.xor
    local.set $complement

    ;; Calculate popcnt(value)
    local.get $value
    i32.popcnt
    local.set $popcnt_val

    ;; Calculate popcnt(~value)
    local.get $complement
    i32.popcnt
    local.set $popcnt_comp

    ;; Sum should equal 32
    local.get $popcnt_val
    local.get $popcnt_comp
    i32.add
    local.set $sum

    local.get $sum
    i32.const 32
    i32.eq)

  ;; Edge case: sparse bit patterns
  (func (export "test_sparse_patterns") (result i32)
    (local $success i32)

    i32.const 1
    local.set $success

    ;; Test 0x80000001 (MSB and LSB set) - 2 bits
    i32.const 0x80000001
    i32.popcnt
    i32.const 2
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0x40000002 (bit 30 and bit 1 set) - 2 bits
    i32.const 0x40000002
    i32.popcnt
    i32.const 2
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Test 0x7FFFFFFE (all except MSB and LSB) - 30 bits
    i32.const 0x7FFFFFFE
    i32.popcnt
    i32.const 30
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    local.get $success)

  ;; Performance test for multiple operations
  (func (export "test_performance") (result i32)
    (local $i i32)
    (local $sum i32)

    ;; Perform 100 i32.popcnt operations to test performance
    i32.const 0
    local.set $i
    i32.const 0
    local.set $sum

    loop $perf_loop
      local.get $i
      i32.popcnt
      local.get $sum
      i32.add
      local.set $sum

      local.get $i
      i32.const 1
      i32.add
      local.set $i

      local.get $i
      i32.const 100
      i32.lt_s
      br_if $perf_loop
    end

    ;; Return accumulated sum as performance indicator
    local.get $sum)
)