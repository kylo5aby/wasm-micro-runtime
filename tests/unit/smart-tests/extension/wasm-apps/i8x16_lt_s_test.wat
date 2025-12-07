(module
  ;; Import memory for test data
  (memory (export "memory") 1)

  ;; Test function for i8x16.lt_s opcode
  ;; Parameters: offsets for loading vectors and storing result
  (func (export "test_i8x16_lt_s") (param $offset_a i32) (param $offset_b i32) (param $offset_result i32)
    ;; Get result address first
    local.get $offset_result

    ;; Load first v128 vector from memory at offset_a
    local.get $offset_a
    v128.load

    ;; Load second v128 vector from memory at offset_b
    local.get $offset_b
    v128.load

    ;; Perform i8x16.lt_s comparison
    i8x16.lt_s

    ;; Store result to memory - stack is [address, result_v128]
    v128.store
  )

  ;; Helper function for basic signed comparison tests
  (func (export "compare_basic") (param $a v128) (param $b v128) (result v128)
    local.get $a
    local.get $b
    i8x16.lt_s
  )

  ;; Helper function for boundary value tests
  (func (export "compare_boundary") (param $a v128) (param $b v128) (result v128)
    local.get $a
    local.get $b
    i8x16.lt_s
  )

  ;; Helper function for identity comparison (anti-reflexive test)
  (func (export "compare_identity") (param $a v128) (result v128)
    local.get $a
    local.get $a
    i8x16.lt_s
  )

  ;; Helper function for zero vector comparisons
  (func (export "compare_with_zero") (param $a v128) (result v128)
    local.get $a
    ;; Create zero vector
    i32.const 0
    i8x16.splat
    i8x16.lt_s
  )

  ;; Test function that validates signed interpretation vs unsigned
  (func (export "signed_vs_unsigned_test") (result v128)
    ;; Create vector with -1 (0xFF) in all lanes
    i32.const -1
    i8x16.splat

    ;; Create vector with 1 in all lanes
    i32.const 1
    i8x16.splat

    ;; Compare: -1 < 1 should be true in signed interpretation
    i8x16.lt_s
  )

  ;; Test function for extreme boundary values
  (func (export "extreme_boundaries_test") (result v128)
    ;; Create vector with INT8_MIN (-128) in all lanes
    i32.const -128
    i8x16.splat

    ;; Create vector with INT8_MAX (127) in all lanes
    i32.const 127
    i8x16.splat

    ;; Compare: -128 < 127 should be true
    i8x16.lt_s
  )

  ;; Test function for mixed positive/negative scenarios
  (func (export "mixed_sign_test") (param $pos_val i32) (param $neg_val i32) (result v128)
    ;; Create vector with negative value in all lanes
    local.get $neg_val
    i8x16.splat

    ;; Create vector with positive value in all lanes
    local.get $pos_val
    i8x16.splat

    ;; Compare: negative < positive should be true
    i8x16.lt_s
  )

  ;; Test function for transitivity validation
  (func (export "transitivity_test") (param $small i32) (param $medium i32) (param $large i32) (result v128)
    ;; Verify small < medium
    local.get $small
    i8x16.splat
    local.get $medium
    i8x16.splat
    i8x16.lt_s

    ;; Result should be all true (0xFF) if small < medium
    drop

    ;; Return comparison of medium < large
    local.get $medium
    i8x16.splat
    local.get $large
    i8x16.splat
    i8x16.lt_s
  )

  ;; Test function for corner case: adjacent values
  (func (export "adjacent_values_test") (result v128)
    ;; Create vector with value 50
    i32.const 50
    i8x16.splat

    ;; Create vector with value 51
    i32.const 51
    i8x16.splat

    ;; Compare: 50 < 51 should be true
    i8x16.lt_s
  )

  ;; Test function for negative adjacent values
  (func (export "negative_adjacent_test") (result v128)
    ;; Create vector with value -51
    i32.const -51
    i8x16.splat

    ;; Create vector with value -50
    i32.const -50
    i8x16.splat

    ;; Compare: -51 < -50 should be true
    i8x16.lt_s
  )

  ;; Test function for wraparound boundary conditions
  (func (export "wraparound_test") (result v128)
    ;; Create vector with -1 (which is 0xFF in unsigned)
    i32.const -1
    i8x16.splat

    ;; Create vector with 0
    i32.const 0
    i8x16.splat

    ;; Compare: -1 < 0 should be true in signed interpretation
    ;; but would be false in unsigned (255 < 0)
    i8x16.lt_s
  )
)