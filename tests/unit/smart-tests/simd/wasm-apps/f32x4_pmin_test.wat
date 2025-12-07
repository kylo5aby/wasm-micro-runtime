;; f32x4_pmin_test.wat - WebAssembly Text Format for f32x4.pmin opcode testing
;; Tests comprehensive f32x4.pmin pseudo-minimum operation functionality

(module
  ;; Import memory for test module requirements
  (memory (export "memory") 1)

  ;; Function: f32x4_pmin_basic
  ;; Description: Performs f32x4.pmin operation on two input f32x4 vectors
  ;; Parameters: Eight f32 parameters (2 f32x4 vectors flattened)
  ;; Returns: Four f32 results (f32x4 result vector flattened)
  (func (export "f32x4_pmin_basic")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)  ;; First f32x4 vector
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)  ;; Second f32x4 vector
    (result f32) (result f32) (result f32) (result f32)              ;; Result f32x4 vector
    (local $vec_result v128)

    ;; Build first vector using splat and replace_lane operations
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    ;; Build second vector using splat and replace_lane operations
    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Perform f32x4.pmin pseudo-minimum operation
    f32x4.pmin
    local.set $vec_result

    ;; Extract individual f32 results from the result vector
    ;; Extract lane 0 result
    (f32x4.extract_lane 0 (local.get $vec_result))
    ;; Extract lane 1 result
    (f32x4.extract_lane 1 (local.get $vec_result))
    ;; Extract lane 2 result
    (f32x4.extract_lane 2 (local.get $vec_result))
    ;; Extract lane 3 result
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Function: f32x4_pmin_boundary
  ;; Description: Tests f32x4.pmin with boundary values (infinity, FLT_MAX, FLT_MIN, subnormals)
  ;; Parameters: Eight f32 parameters for boundary value testing
  ;; Returns: Four f32 results
  (func (export "f32x4_pmin_boundary")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)
    (result f32) (result f32) (result f32) (result f32)
    (local $vec_result v128)

    ;; Build first vector for boundary testing
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    ;; Build second vector for boundary testing
    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Apply f32x4.pmin for boundary value pseudo-minimum
    f32x4.pmin
    local.set $vec_result

    ;; Extract boundary test results
    (f32x4.extract_lane 0 (local.get $vec_result))
    (f32x4.extract_lane 1 (local.get $vec_result))
    (f32x4.extract_lane 2 (local.get $vec_result))
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Function: f32x4_pmin_nan
  ;; Description: Tests f32x4.pmin NaN handling (key pseudo-minimum behavior)
  ;; Parameters: Eight f32 parameters including NaN values
  ;; Returns: Four f32 results with proper NaN handling
  (func (export "f32x4_pmin_nan")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)
    (result f32) (result f32) (result f32) (result f32)
    (local $vec_result v128)

    ;; Build first vector with NaN values for testing
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    ;; Build second vector with NaN values for testing
    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Apply f32x4.pmin with NaN pseudo-minimum rules
    f32x4.pmin
    local.set $vec_result

    ;; Extract NaN handling test results
    (f32x4.extract_lane 0 (local.get $vec_result))
    (f32x4.extract_lane 1 (local.get $vec_result))
    (f32x4.extract_lane 2 (local.get $vec_result))
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Function: f32x4_pmin_zero
  ;; Description: Tests f32x4.pmin signed zero handling (+0.0f vs -0.0f)
  ;; Parameters: Eight f32 parameters focusing on signed zero behavior
  ;; Returns: Four f32 results with proper signed zero handling
  (func (export "f32x4_pmin_zero")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)
    (result f32) (result f32) (result f32) (result f32)
    (local $vec_result v128)

    ;; Build first vector with signed zero values
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    ;; Build second vector with signed zero values
    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Apply f32x4.pmin with signed zero pseudo-minimum
    f32x4.pmin
    local.set $vec_result

    ;; Extract signed zero handling results
    (f32x4.extract_lane 0 (local.get $vec_result))
    (f32x4.extract_lane 1 (local.get $vec_result))
    (f32x4.extract_lane 2 (local.get $vec_result))
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Helper function: test_predefined_pmin
  ;; Description: Tests f32x4.pmin with predefined test vectors for verification
  ;; Returns: Four f32 results from predefined input combinations
  (func (export "test_predefined_pmin") (result f32) (result f32) (result f32) (result f32)
    (local $result v128)
    ;; Test: pmin([1.5, 2.0, 3.5, 4.0], [2.5, 1.5, 4.0, 3.0])
    ;; Expected result: [1.5, 1.5, 3.5, 3.0]

    ;; First vector: [1.5, 2.0, 3.5, 4.0]
    (f32x4.splat (f32.const 1.5))
    (f32x4.replace_lane 1 (f32.const 2.0))
    (f32x4.replace_lane 2 (f32.const 3.5))
    (f32x4.replace_lane 3 (f32.const 4.0))

    ;; Second vector: [2.5, 1.5, 4.0, 3.0]
    (f32x4.splat (f32.const 2.5))
    (f32x4.replace_lane 1 (f32.const 1.5))
    (f32x4.replace_lane 2 (f32.const 4.0))
    (f32x4.replace_lane 3 (f32.const 3.0))

    ;; Apply f32x4.pmin pseudo-minimum operation
    f32x4.pmin
    local.set $result

    ;; Extract and return predefined test results
    (f32x4.extract_lane 0 (local.get $result))
    (f32x4.extract_lane 1 (local.get $result))
    (f32x4.extract_lane 2 (local.get $result))
    (f32x4.extract_lane 3 (local.get $result))
  )

  ;; Helper function: test_infinity_pmin
  ;; Description: Tests f32x4.pmin with infinity combinations
  ;; Returns: Four f32 results from infinity test cases
  (func (export "test_infinity_pmin") (result f32) (result f32) (result f32) (result f32)
    (local $result v128)
    ;; Test: pmin([+INF, -INF, +INF, 1.0], [1.0, 1.0, -INF, +INF])
    ;; Expected result: [1.0, -INF, -INF, 1.0]

    ;; First vector: [+INF, -INF, +INF, 1.0]
    (f32x4.splat (f32.const inf))
    (f32x4.replace_lane 1 (f32.const -inf))
    (f32x4.replace_lane 2 (f32.const inf))
    (f32x4.replace_lane 3 (f32.const 1.0))

    ;; Second vector: [1.0, 1.0, -INF, +INF]
    (f32x4.splat (f32.const 1.0))
    (f32x4.replace_lane 1 (f32.const 1.0))
    (f32x4.replace_lane 2 (f32.const -inf))
    (f32x4.replace_lane 3 (f32.const inf))

    ;; Apply f32x4.pmin for infinity pseudo-minimum
    f32x4.pmin
    local.set $result

    ;; Extract infinity test results
    (f32x4.extract_lane 0 (local.get $result))
    (f32x4.extract_lane 1 (local.get $result))
    (f32x4.extract_lane 2 (local.get $result))
    (f32x4.extract_lane 3 (local.get $result))
  )

  ;; Helper function: test_nan_behavior_pmin
  ;; Description: Tests f32x4.pmin NaN behavior (non-NaN selection priority)
  ;; Returns: Four f32 results demonstrating NaN pseudo-minimum rules
  (func (export "test_nan_behavior_pmin") (result f32) (result f32) (result f32) (result f32)
    (local $result v128)
    ;; Test: pmin([NaN, 1.0, 2.0, NaN], [2.0, NaN, 3.0, 5.0])
    ;; Expected result: [2.0, 1.0, 2.0, 5.0] (non-NaN values selected)

    ;; First vector: [NaN, 1.0, 2.0, NaN]
    (f32x4.splat (f32.const nan:0x200000))
    (f32x4.replace_lane 1 (f32.const 1.0))
    (f32x4.replace_lane 2 (f32.const 2.0))
    (f32x4.replace_lane 3 (f32.const nan:0x200000))

    ;; Second vector: [2.0, NaN, 3.0, 5.0]
    (f32x4.splat (f32.const 2.0))
    (f32x4.replace_lane 1 (f32.const nan:0x200000))
    (f32x4.replace_lane 2 (f32.const 3.0))
    (f32x4.replace_lane 3 (f32.const 5.0))

    ;; Apply f32x4.pmin for NaN pseudo-minimum behavior
    f32x4.pmin
    local.set $result

    ;; Extract NaN behavior test results
    (f32x4.extract_lane 0 (local.get $result))
    (f32x4.extract_lane 1 (local.get $result))
    (f32x4.extract_lane 2 (local.get $result))
    (f32x4.extract_lane 3 (local.get $result))
  )
)