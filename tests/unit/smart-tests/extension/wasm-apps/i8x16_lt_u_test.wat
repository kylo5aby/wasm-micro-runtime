(module
  ;; Import memory for test data
  (memory (export "memory") 1)

  ;; Test function for i8x16.lt_u opcode
  ;; Parameters: offsets for loading vectors and storing result
  (func (export "test_i8x16_lt_u") (param $offset_a i32) (param $offset_b i32) (param $offset_result i32)
    ;; Get result address first
    local.get $offset_result

    ;; Load first v128 vector from memory at offset_a
    local.get $offset_a
    v128.load

    ;; Load second v128 vector from memory at offset_b
    local.get $offset_b
    v128.load

    ;; Perform i8x16.lt_u comparison
    i8x16.lt_u

    ;; Store result to memory - stack is [address, result_v128]
    v128.store
  )

  ;; Helper function for basic unsigned comparison tests
  (func (export "compare_basic") (param $a v128) (param $b v128) (result v128)
    local.get $a
    local.get $b
    i8x16.lt_u
  )

  ;; Helper function for boundary value tests
  (func (export "compare_boundary") (param $a v128) (param $b v128) (result v128)
    local.get $a
    local.get $b
    i8x16.lt_u
  )

  ;; Helper function for identity comparison (anti-reflexive test)
  (func (export "compare_identity") (param $a v128) (result v128)
    local.get $a
    local.get $a
    i8x16.lt_u
  )

  ;; Helper function for zero vector comparisons
  (func (export "compare_with_zero") (param $a v128) (result v128)
    local.get $a
    ;; Create zero vector
    i32.const 0
    i8x16.splat
    i8x16.lt_u
  )

  ;; Test function that validates unsigned interpretation vs signed
  (func (export "unsigned_vs_signed_test") (result v128)
    ;; Create vector with 255 (0xFF) in all lanes - max unsigned, -1 signed
    i32.const 255
    i8x16.splat

    ;; Create vector with 1 in all lanes
    i32.const 1
    i8x16.splat

    ;; Compare: 255 < 1 should be false in unsigned interpretation
    ;; (but would be true in signed: -1 < 1)
    i8x16.lt_u
  )

  ;; Test function for extreme boundary values
  (func (export "extreme_boundaries_test") (result v128)
    ;; Create vector with 0 (unsigned minimum) in all lanes
    i32.const 0
    i8x16.splat

    ;; Create vector with 255 (unsigned maximum) in all lanes
    i32.const 255
    i8x16.splat

    ;; Compare: 0 < 255 should be true in unsigned
    i8x16.lt_u
  )

  ;; Test function for mixed value scenarios
  (func (export "mixed_values_test") (param $small_val i32) (param $large_val i32) (result v128)
    ;; Create vector with small value in all lanes
    local.get $small_val
    i8x16.splat

    ;; Create vector with large value in all lanes
    local.get $large_val
    i8x16.splat

    ;; Compare: small < large should be true
    i8x16.lt_u
  )

  ;; Test function for transitivity validation
  (func (export "transitivity_test") (param $small i32) (param $medium i32) (param $large i32) (result v128)
    ;; Verify small < medium
    local.get $small
    i8x16.splat
    local.get $medium
    i8x16.splat
    i8x16.lt_u

    ;; Result should be all true (0xFF) if small < medium
    drop

    ;; Return comparison of medium < large
    local.get $medium
    i8x16.splat
    local.get $large
    i8x16.splat
    i8x16.lt_u
  )

  ;; Test function for corner case: adjacent values
  (func (export "adjacent_values_test") (result v128)
    ;; Create vector with value 100
    i32.const 100
    i8x16.splat

    ;; Create vector with value 101
    i32.const 101
    i8x16.splat

    ;; Compare: 100 < 101 should be true
    i8x16.lt_u
  )

  ;; Test function for high-range adjacent values
  (func (export "high_range_adjacent_test") (result v128)
    ;; Create vector with value 254
    i32.const 254
    i8x16.splat

    ;; Create vector with value 255
    i32.const 255
    i8x16.splat

    ;; Compare: 254 < 255 should be true
    i8x16.lt_u
  )

  ;; Test function for cross-boundary conditions (signed vs unsigned critical point)
  (func (export "cross_boundary_test") (result v128)
    ;; Create vector with 127 (max positive signed)
    i32.const 127
    i8x16.splat

    ;; Create vector with 128 (min negative signed, but 128 unsigned)
    i32.const 128
    i8x16.splat

    ;; Compare: 127 < 128 should be true in unsigned interpretation
    ;; (127 < -128 would be false in signed)
    i8x16.lt_u
  )

  ;; Test function for maximum unsigned boundary
  (func (export "max_unsigned_boundary_test") (result v128)
    ;; Create vector with 254
    i32.const 254
    i8x16.splat

    ;; Create vector with 0 (wraparound test)
    i32.const 0
    i8x16.splat

    ;; Compare: 254 < 0 should be false (no wraparound in unsigned comparison)
    i8x16.lt_u
  )

  ;; Test function for comprehensive unsigned ordering
  (func (export "unsigned_ordering_test") (param $val1 i32) (param $val2 i32) (result v128)
    ;; Create vectors with specified values
    local.get $val1
    i8x16.splat
    local.get $val2
    i8x16.splat

    ;; Perform unsigned comparison
    i8x16.lt_u
  )

  ;; Test function for all-equal comparison (should always be false)
  (func (export "all_equal_test") (param $val i32) (result v128)
    ;; Create identical vectors
    local.get $val
    i8x16.splat
    local.get $val
    i8x16.splat

    ;; Compare: val < val should always be false (anti-reflexive)
    i8x16.lt_u
  )

  ;; Test function for power-of-two boundary testing
  (func (export "power_of_two_test") (result v128)
    ;; Create vector with 64
    i32.const 64
    i8x16.splat

    ;; Create vector with 128
    i32.const 128
    i8x16.splat

    ;; Compare: 64 < 128 should be true
    i8x16.lt_u
  )
)