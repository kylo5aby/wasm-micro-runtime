(module
  ;; Basic i32.rotr test function - takes two i32 parameters and returns rotated result
  (func (export "i32_rotr_test") (param $value i32) (param $count i32) (result i32)
    local.get $value
    local.get $count
    i32.rotr)

  ;; Single parameter test function for edge case testing
  (func (export "test_basic_rotr") (param $input i32) (result i32)
    local.get $input
    i32.const 4  ;; Fixed rotation count for basic testing
    i32.rotr)

  ;; Zero rotation test - should return original value
  (func (export "test_zero_rotation") (param $input i32) (result i32)
    local.get $input
    i32.const 0
    i32.rotr)

  ;; Full rotation test - rotate by 32 should return original value
  (func (export "test_full_rotation") (param $input i32) (result i32)
    local.get $input
    i32.const 32
    i32.rotr)

  ;; Boundary rotation test - rotate by 31
  (func (export "test_boundary_rotation") (param $input i32) (result i32)
    local.get $input
    i32.const 31
    i32.rotr)

  ;; Large count rotation test - test modulo behavior
  (func (export "test_large_count") (param $input i32) (result i32)
    local.get $input
    i32.const 33  ;; Should be equivalent to rotation by 1
    i32.rotr)

  ;; Mathematical property test - rotate twice to verify consistency
  (func (export "test_double_rotation") (param $value i32) (param $count1 i32) (param $count2 i32) (result i32)
    local.get $value
    local.get $count1
    i32.rotr
    local.get $count2
    i32.rotr)

  ;; Powers of 2 test function
  (func (export "test_power_of_2") (result i32)
    i32.const 1    ;; Test rotating single bit
    i32.const 1
    i32.rotr)

  ;; All ones test function
  (func (export "test_all_ones") (result i32)
    i32.const 0xffffffff
    i32.const 7
    i32.rotr)

  ;; Alternating pattern test
  (func (export "test_alternating_pattern") (result i32)
    i32.const 0x55555555
    i32.const 1
    i32.rotr)

  ;; Complex validation function for multiple test cases
  (func (export "validate_rotr_properties") (result i32)
    (local $test_val i32)
    (local $temp i32)
    (local $success i32)

    ;; Initialize success flag
    i32.const 1
    local.set $success

    ;; Test value for property validation
    i32.const 0x12345678
    local.set $test_val

    ;; Property 1: rotr(x, 0) == x
    local.get $test_val
    i32.const 0
    i32.rotr
    local.get $test_val
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 2: rotr(x, 32) == x
    local.get $test_val
    i32.const 32
    i32.rotr
    local.get $test_val
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 3: rotr(0xffffffff, any) == 0xffffffff
    i32.const 0xffffffff
    i32.const 13
    i32.rotr
    i32.const 0xffffffff
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 4: rotr(0, any) == 0
    i32.const 0
    i32.const 25
    i32.rotr
    i32.const 0
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Property 5: Test modulo behavior - rotr(x, 33) == rotr(x, 1)
    local.get $test_val
    i32.const 33
    i32.rotr
    local.set $temp

    local.get $test_val
    i32.const 1
    i32.rotr
    local.get $temp
    i32.ne
    if
      i32.const 0
      local.set $success
    end

    ;; Return success flag (1 if all tests pass, 0 if any fail)
    local.get $success)

  ;; Performance test function for benchmarking
  (func (export "performance_test") (param $iterations i32) (result i32)
    (local $i i32)
    (local $result i32)

    i32.const 0
    local.set $i
    i32.const 0x12345678
    local.set $result

    (loop $loop
      local.get $result
      i32.const 7
      i32.rotr
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
)